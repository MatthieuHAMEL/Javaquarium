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
  int healthPoints;
  unsigned age; // in number of turns
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
struct SpeciesR{}; // Relationship symbol
struct Bass{}; struct Tuna{}; struct ClownFish{}; struct Grouper{}; struct Sole{}; struct Carp{};

// The feeding behaviors
struct FeedingR{};
struct Herbivorous{}; struct Carnivorous{};

// Traits mapping specy -> behaviors
template<typename T> struct Characteristics{};
template<> struct Characteristics<Bass> { using feeding = Carnivorous; };
template<> struct Characteristics<Tuna> { using feeding = Carnivorous; };
template<> struct Characteristics<ClownFish> { using feeding = Carnivorous; };
template<> struct Characteristics<Grouper> { using feeding = Herbivorous; };
template<> struct Characteristics<Sole> { using feeding = Herbivorous; };
template<> struct Characteristics<Carp> { using feeding = Herbivorous; }; // TODO replace all of this by is_a (component inheritance)
// (Carp prefab is_a<Herbivorous>) -> adding a carp == adding a Carp with Herbivorous
// Still, there is the need to construct one prefab for each fish specy.

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

class Aquarium final
{
public:
  Aquarium() :
    m_seaweedQ {m_world.query_builder<Seaweed>().build()},
    m_fishQ {m_world.query_builder<Fish>().build()}
  {
    m_world.component<Living>().add(flecs::OnInstantiate, flecs::Override);
    m_livingPrefab = m_world.prefab().set<Living>({ .healthPoints = 10 });

    // Ageing: all living beings get older at every turn
    // It doesn't need to be immediate() since this system's logic doesn't rely on previous fishes being actually destroyed.
    m_world.system<Living>()
      .each([this](flecs::entity e, Living& living) {
        if (++living.age >= Living::MAX_AGE) {
          if (e.has<Fish>()) {
            cout << e.get<Fish>().name << " has died from old age!\n";
          }
          immediateDestroy(e);
        }
      });

    // All seaweed get 1 HP at each turn
    m_world.system<Seaweed, Living>()
      .each([](Seaweed const&, Living& living) {
        changeHealth(living, 1);
      });
    
    m_world.system<Fish, Living>()
      .immediate() // cf. defer_suspend. Don't queue an entity destruction here!
      .each([this](flecs::entity predator, Fish const& f, Living& living)
      {
        // The fish gets hungry at every turn: lose one HP
        if (changeHealth(living, -1)) {
          cout << f.name << " is dead because it was too hungry!\n";
          immediateDestroy(predator);
          return;
        }

        if (!isHungry(living)) {
          return;
        }
        
        // Now try to eat
        if (predator.has<FeedingR, Herbivorous>()) {
          // Look for a seaweed to eat
          m_seaweedQ.run([&](flecs::iter& it) {
            if (it.next()) {
              auto victim = it.entity(0); // The first seaweed I find
              if (changeHealth(victim.get_mut<Living>(), -2)) { // Seaweed HP has reached 0
                immediateDestroy(victim);
              }
              // The predator gets +3 HPs for eating a seaweed.
              changeHealth(living, 3);
            } else {
              cout << f.name << " can't find any seaweed!\n";
            }
          });
        }
        else { // Carnivorous fishes eat other fishes
          assert((predator.has<FeedingR, Carnivorous>()));
          
          if (m_world.count<Fish>() == 1) { // There's only one remaining fish in the aquarium
            cout << f.name << " is the only fish left! It can't eat!\n";
            return; // Otherwise the below do..while will never end!
          }

          flecs::entity victim;
          do {
            victim = random_fish(m_fishQ);
          } while(victim == predator); // the predator shouldn't eat itself (nor try to do so)

          if (victim.target<SpeciesR>() == predator.target<SpeciesR>()) {
            cout << f.name << " wanted to eat " << victim.get<Fish>().name << " but they are of the same specy!\n";
            return; // There is no other chance.
          }
          cout << f.name << " is eating " << victim.get<Fish>().name << '\n';
          if (changeHealth(victim.get_mut<Living>(), -4)) { // The victim is dead
            cout << victim.get<Fish>().name << " is dead... RIP\n";
            immediateDestroy(victim);
          }
          changeHealth(living, 5); // The predator gets HPs
        }
      });
  }
  
  template<typename SPECY>
  Aquarium& add_fish(std::string name, Sex sex, unsigned age=0) {
    using Feeding = typename Characteristics<SPECY>::feeding;
    
    auto e = m_world.entity(name.c_str()).set<Fish>({std::move(name), sex})
      .is_a(m_livingPrefab) // (TODO) what about adding the "<flecs::With, Living>" trait on Fish and Seaweed?
      .add<SpeciesR, SPECY>() // Tuna, Bass, ClownFish...
      .template add<FeedingR, Feeding>(); // Herbivorous/Carnivorous
    e.template get_mut<Living>().age = age;
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
           << "(" << e.target<SpeciesR>().name() << ") (" << living.healthPoints << " HP)\n";
    });
    m_world.each([](Seaweed const&, Living& living) {
      cout << "\t- Seaweed (" << living.healthPoints << " HP)\n";
    });
  }
private:
  // Must be called from an immediate system. Otherwise it won't be immediate.
  void immediateDestroy(flecs::entity iEntity) {
    m_world.defer_suspend();
    iEntity.destruct();
    m_world.defer_resume();
  }

  flecs::world m_world;
  flecs::entity m_livingPrefab;
  flecs::query<Seaweed> m_seaweedQ;
  flecs::query<Fish> m_fishQ;
};

auto main() -> int {
  Aquarium aq;
  aq.add_fish<Bass>("Bob", Sex::M, /*age*/ 10)
    .add_fish<Tuna>("Brenda", Sex::F)
    .add_fish<Grouper>("Rhonda", Sex::F)
    .add_fish<ClownFish>("Francesca", Sex::F, /*age*/ 4)
    .add_fish<Tuna>("Gerard", Sex::M)
    .add_fish<Sole>("Samantha", Sex::F)
    .add_seaweed(5);

  println("----- Initial state -----");
  aq.printAquarium();
  
  for (size_t i = 1; i < 40; ++i) {
    println("----- Step {} -----", i);
    aq.tick();
  }
  return 0;
}
