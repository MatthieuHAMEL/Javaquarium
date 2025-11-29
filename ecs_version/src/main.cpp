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
};

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
    m_world.component<Living>().add(flecs::OnInstantiate, flecs::Inherit);
    m_livingPrefab = m_world.prefab().set<Living>({ .healthPoints = 10 });
    
    m_world.system<Fish>()
      // enables defer_suspend so that the victim is really destroyed.
      // Otherwise several fishes eat the same seaweed because of flecs commands queuing.
      .immediate()
      .each([this](flecs::entity predator, Fish const& f)
      {
        if (predator.has<FeedingR, Herbivorous>()) {
          // Look for a seaweed to eat
          m_seaweedQ.run([&](flecs::iter& it) {
            if (it.next()) {
              auto victim = it.entity(0); // The first seaweed I find
              m_world.defer_suspend();
              victim.destruct(); // yum yum
              m_world.defer_resume();
            } else {
              cout << f.name << " can't find any seaweed!\n";
            }
          });
        }
        else { // Carnivorous fishes eat other fishes
          assert((predator.has<FeedingR, Carnivorous>()));
          if (m_world.count<Fish>() == 1) { // There's only one remaining fish in the aquarium
            cout << "One fish left! " << f.name << " can't eat!\n";
            return; // Otherwise the below do..while will never end!
          }

          flecs::entity victim;
          do {
            victim = random_fish(m_fishQ);
          } while(victim == predator); // the predator shouldn't eat itself!
          cout << f.name << " is eating " << victim.get<Fish>().name << '\n';
          m_world.defer_suspend();
          victim.destruct();
          m_world.defer_resume();
        }
      });
  }
  
  template<typename SPECY>
  Aquarium& add_fish(std::string name, Sex sex) {
    using Feeding = typename Characteristics<SPECY>::feeding;
    
    m_world.entity(name.c_str()).set<Fish>({std::move(name), sex})
      .is_a(m_livingPrefab)
      .add<SpeciesR, SPECY>() // Tuna, Bass, ClownFish...
      .template add<FeedingR, Feeding>(); // Herbivorous/Carnivorous
    return *this;
  }
  Aquarium& add_seaweed(size_t nb = 1) {
    for (size_t i = 0; i < nb; ++i) {
      m_world.entity().add<Seaweed>().is_a(m_livingPrefab);
    }
    return *this;
  }

  void tick() {
    m_world.progress(); // runs the systems
    printAquarium();
  }

  void printAquarium() {
    println("There are {} seaweeds and {} fishes:", m_world.count<Seaweed>(), m_world.count<Fish>());
    m_world.each<Fish>([](flecs::entity e, Fish const& f) {
      cout << "\t- " << f.name << "(" << toChar(f.sex) << ") "
           << "(" << e.target<SpeciesR>().name() << ")\n";
    });
  }
private:
  flecs::world m_world;
  flecs::entity m_livingPrefab;
  flecs::query<Seaweed> m_seaweedQ;
  flecs::query<Fish> m_fishQ;
};

auto main() -> int {
  Aquarium aq;
  aq.add_fish<Bass>("Bob", Sex::M)
    .add_fish<Tuna>("Brenda", Sex::F)
    .add_fish<Grouper>("Rhonda", Sex::F)
    .add_fish<ClownFish>("Francesca", Sex::F)
    .add_fish<Tuna>("Gerard", Sex::M)
    .add_fish<Sole>("Samantha", Sex::F)
    .add_seaweed(10);

  println("----- Initial state -----");
  aq.printAquarium();
  
  for (size_t i = 1; i < 15; ++i) {
    println("----- Step {} -----", i);
    aq.tick();
  }
  return 0;
}
