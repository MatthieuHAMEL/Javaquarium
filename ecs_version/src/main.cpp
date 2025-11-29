#include "flecs.h"
import std;
using namespace std;

enum class Sex { F, M };
char toChar(Sex s) {
  return s == Sex::F ? 'F' : 'M';
}

struct Fish {
  std::string name;
  Sex sex;
};
struct Seaweed{};

struct Living {
  int healthPoints = 10;
  unsigned age = 0; // in number of turns
  static constexpr unsigned MAX_AGE = 20;
};

// Returns whether the Living is dead or not
bool changeHealth(Living& iLiving, int delta) {
  iLiving.healthPoints += delta;
  return (iLiving.healthPoints <= 0);
}

// Applies only to a fish
bool isHungry(Living const& iLiving) {
  assert(iLiving.healthPoints > 0);
  return iLiving.healthPoints <= 5;
}

// The fish species
enum class Species : size_t { Bass, Tuna, ClownFish, Grouper, Sole, Carp };

const char* toString(Species species) {
  switch (species) {
  case Species::Bass:
    return "Bass";
  case Species::Tuna:
    return "Tuna";
  case Species::ClownFish:
    return "ClownFish";
  case Species::Grouper:
    return "Grouper";
  case Species::Sole:
    return "Sole";
  case Species::Carp:
    return "Carp";
  }
  assert(false);
}

struct SpeciesInfo{Species id;}; // To store as a component to test whether two fishes have the same specy

// The feeding behaviors
struct FeedingR{};
struct Herbivorous{}; struct Carnivorous{};

std::mt19937 rng{std::random_device{}()};

flecs::entity random_fish(flecs::query<Fish>& fishQ) {
  flecs::entity result;   // null by default
  size_t seen = 0;

  // Reservoir sampling
  fishQ.each([&](flecs::entity e, Fish const&) {
    ++seen;
    std::uniform_int_distribution<std::size_t> dist(1, seen);
    if (dist(rng) == 1) {
      result = e;
    }
  });
  
  return result;
}

std::string babyName(std::string const& firstParentName, std::string const& secondParentName) {
  std::array prefixes {"Lil'", "Little", "Op'", "Mini", ""};
  std::array suffixes {"Junior", "boo", "da", "dee", "by"};

  std::uniform_int_distribution<std::size_t> distPrefix(0, prefixes.size() - 1);
  std::uniform_int_distribution<std::size_t> distSuffix(0, suffixes.size() - 1);
  std::bernoulli_distribution chooseParent(0.5); // 50/50 chance
  const std::string& parentName = chooseParent(rng) ? firstParentName : secondParentName;

  return prefixes[distPrefix(rng)] + parentName + suffixes[distSuffix(rng)];
}


constexpr unsigned NB_SPECIES = 6;

class Aquarium final
{
public:
  Aquarium() :
    m_livingPrefab{m_world.prefab().add<Living>()},
    m_seaweedQ {m_world.query_builder<Seaweed>().build()},
    m_fishQ {m_world.query_builder<Fish>().build()}
  {
    make_species_prefab(Species::Bass, /*isCarnivorous*/true);
    make_species_prefab(Species::Tuna, true);
    make_species_prefab(Species::ClownFish, true);
    make_species_prefab(Species::Grouper, false);
    make_species_prefab(Species::Sole, false);
    make_species_prefab(Species::Carp, false);

    ////////////////// SYSTEMS //////////////////////////
    // Ageing: all living beings get older at every turn
    // It doesn't need to be immediate() since this system's logic doesn't rely on previous fishes being actually destroyed.
    m_world.system<Living>().immediate()
      .each([this](flecs::entity e, Living& living) {
        if (++living.age >= Living::MAX_AGE) {
          if (e.has<Fish>()) {
            cout << e.get<Fish>().name << " has died from old age!" << endl;
          }
          immediateDestroy(e);
        }
      });

    /////////////////////////////////////////////////////

    // All seaweed get 1 HP at each turn
    m_world.system<Seaweed, Living>()
      .each([](Seaweed const&, Living& living) {
        changeHealth(living, 1);
      });
    
    /////////////////////////////////////////////////////

    // The fish life
    m_world.system<Fish, Living>()
      .immediate() // cf. defer_suspend. Don't queue an entity destruction here!
      .each([this](flecs::entity curFish, Fish const& f, Living& living)
      {
        cout << "my name is " << f.name << endl;
        // The fish gets hungry at every turn: lose one HP
        if (changeHealth(living, -1)) {
          cout << f.name << " is dead because it was too hungry!" << endl;
          immediateDestroy(curFish);
          return;
        }

        if (!isHungry(living)) {
          // Non-hungry fishes will try to have a baby fish with another fish
          if (auto lover = tryToHaveABaby(curFish, f)) {
            cout << f.name << " had a baby with " << lover.get<Fish>().name << "!\n";
          }
          return;
        }
        
        // Now try to eat
        if (curFish.has<FeedingR, Herbivorous>()) {
          // Look for a seaweed to eat
          m_seaweedQ.run([&](flecs::iter& it) {
            if (it.next()) {
              auto victim = it.entity(0); // The first seaweed I find
              changeHealth(living, 3); // The "predator" gets +3 HPs for eating a seaweed.
              if (changeHealth(victim.get_mut<Living>(), -2)) { // Seaweed HP has reached 0
                immediateDestroy(victim);
                return;
              }
            } else {
              cout << f.name << " can't find any seaweed!\n";
            }
          });
        }
        else { // Carnivorous fishes eat other fishes
          assert((curFish.has<FeedingR, Carnivorous>()));
          
          if (m_world.count<Fish>() == 1) { // There's only one remaining fish in the aquarium
            cout << f.name << " is the only fish left! It can't eat!\n";
            return; // Otherwise the below do..while will never end!
          }

          flecs::entity victim;
          do {
            victim = random_fish(m_fishQ);
          } while(victim == curFish); // the predator shouldn't eat itself (nor try to do so)

          if (victim.get<SpeciesInfo>().id == curFish.get<SpeciesInfo>().id) {
            cout << f.name << " wanted to eat " << victim.get<Fish>().name << " but they are of the same specy!\n";
            return; // There is no other chance.
          }
          cout << f.name << " is eating " << victim.get<Fish>().name << '\n';
          changeHealth(living, 5); // The predator gets HPs
          if (changeHealth(victim.get_mut<Living>(), -4)) { // The victim is dead
            cout << victim.get<Fish>().name << " is dead... RIP\n";
            immediateDestroy(victim);
          }
        }
      });
  }
  
  Aquarium& add_fish(Species species, std::string name, Sex sex, unsigned age=0) {
    auto speciesPrefab = m_speciesPrefabs[static_cast<size_t>(species)];
    auto e = m_world.entity(name.c_str())
      .is_a(speciesPrefab) // inherits Living + appropriate feeding behavior
      .set<Fish>({std::move(name), sex})
      .set<Living>({.age = age}); // healthpoints are by default (10)
    return *this;
  }
  Aquarium& add_seaweed(size_t nb = 1) {
    for (size_t i = 0; i < nb; ++i) {
      m_world.entity().add<Seaweed>().is_a(m_livingPrefab);
    }
    return *this;
  }
  Aquarium& add_seaweed_with_age(unsigned age) {
    auto e = m_world.entity().add<Seaweed>().is_a(m_livingPrefab);
    e.get_mut<Living>().age = age;
    return *this;
  }

  void tick() {
    m_world.progress(); // runs the systems
    printAquarium();
  }

  void printAquarium() {
    println("There are {} seaweeds and {} fishes:", m_world.count<Seaweed>(), m_world.count<Fish>());
    m_world.each([](flecs::entity e, Fish const& f, Living const& living) {
      cout << "\t- " << f.name << "(" << toChar(f.sex) << ") "
           << "(" << toString(e.get<SpeciesInfo>().id) << ") (" << living.healthPoints << " HP) (Age " << living.age << ")\n";
    });
    m_world.each([](Seaweed const&, Living& living) {
      cout << "\t- Seaweed (" << living.healthPoints << " HP)\n";
    });
  }
private:
  void make_species_prefab(Species id, bool isCarnivorous) { // At construction
    assert(static_cast<size_t>(id) < NB_SPECIES);
    
    auto prefab = m_world.prefab().is_a(m_livingPrefab).set<SpeciesInfo>({id});
    if (isCarnivorous) {
      prefab.add<FeedingR, Carnivorous>();
    }
    else {
      prefab.add<FeedingR, Herbivorous>();
    }
    m_speciesPrefabs[static_cast<size_t>(id)] = prefab;
  }

  // Must be called from an immediate system. Otherwise it won't be immediate.
  void immediateDestroy(flecs::entity iEntity) {
    m_world.defer_suspend();
    iEntity.destruct();
    m_world.defer_resume();
  }

  // Return the lover id if it worked.
  flecs::entity tryToHaveABaby(flecs::entity curFish, Fish const& curFishCompo) {
    if (m_world.count<Fish>() == 1) { // There's only one remaining fish in the aquarium
      return flecs::entity::null(); // Otherwise the below do..while will never end!
    }

    flecs::entity possibleLover;
    do {
      possibleLover = random_fish(m_fishQ);
    } while(possibleLover == curFish); // the current fish will not date itself

    // Same specy, different sex -> let's have a baby
    if ((possibleLover.get<SpeciesInfo>().id == curFish.get<SpeciesInfo>().id)
        && possibleLover.get<Fish>().sex != curFishCompo.sex) {

      add_fish(possibleLover.get<SpeciesInfo>().id, // Same species than both parents
               babyName(curFish.get<Fish>().name, possibleLover.get<Fish>().name), // cute name generated from one of the parents name
               Sex::F); // TODO random Sex
      return possibleLover;
    }
    
    return flecs::entity::null(); // Else it didn't work :-(
  }
  flecs::world m_world;
  flecs::entity m_livingPrefab;
  flecs::query<Seaweed> m_seaweedQ;
  flecs::query<Fish> m_fishQ;
  std::array<flecs::entity, NB_SPECIES> m_speciesPrefabs; // In the order of the Species enum
};

auto main() -> int {
  Aquarium aq;
  aq.add_fish(Species::Bass, "Bob", Sex::M, /*age*/ 10)
    .add_fish(Species::Tuna, "Brenda", Sex::F)
    .add_fish(Species::Grouper, "Rhonda", Sex::F)
    .add_fish(Species::ClownFish, "Francesca", Sex::F, /*age*/ 4)
    .add_fish(Species::Tuna, "Gerard", Sex::M)
    .add_fish(Species::Sole, "Samantha", Sex::F)
    .add_seaweed(5);

  println("----- Initial state -----");
  aq.printAquarium();
  
  for (size_t i = 1; i < 40; ++i) {
    println("----- Step {} -----", i);
    aq.tick();
  }
  return 0;
}
