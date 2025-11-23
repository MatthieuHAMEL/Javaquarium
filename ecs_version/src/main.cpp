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

// Gives an unskewed distribution. uBound included
int random_int(int lBound, int uBound) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(lBound, uBound);
  return distrib(gen);
}

class Aquarium final
{
public:
  Aquarium() : m_seaweedQ {m_world.query_builder<Seaweed>().build()} {
    // Herbivorous fishes eat seaweeds
    m_world.system<Fish>().with<FeedingR, Herbivorous>()
      // enables defer_suspend so that the victim is really destroyed.
      // Otherwise several fishes eat the same seaweed because of flecs commands queuing.
      .immediate()
      .each([this](Fish const& f) {
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
      });

    // Carnivorous fishes eat other fishes
    m_world.system<Fish>().with<FeedingR, Carnivorous>().immediate()
      .run([](flecs::iter& it) {
        while (it.next()) {
          cout << "COUNT OF CARN ITER: " << it.count() << endl;
          // if (it.count() == 1) {
          //   cout << "returning 68 one fish\n";
          //   return; // Otherwise the below do..while will never end!
          // }
          unsigned w=0;
          for(auto i : it) {
            w++;
            flecs::entity predator = it.entity(i);
            cout << "hi from predator: " << predator.get<Fish>().name << endl;
            flecs::entity victim;
            do {
              int randomIdx = random_int(0, static_cast<int>(it.count() - 1));
              victim = it.entity(randomIdx);
            } while(victim == predator); // the predator shouldn't eat itself
            cout << predator.get<Fish>().name << " is eating " << victim.get<Fish>().name << '\n';
            it.world().defer_suspend();
            victim.destruct();
            it.world().defer_resume();
          }

          cout << "COUNT BYLOOP : " << w << endl;
        }
      });
  }
  template<typename SPECY>
  Aquarium& add_fish(std::string name, Sex sex) {
    using Feeding = typename Characteristics<SPECY>::feeding;
    
    m_world.entity(name.c_str()).set<Fish>({std::move(name), sex})
      .add<SpeciesR, SPECY>() // Tuna, Bass, ClownFish...
      .template add<FeedingR, Feeding>(); // Herbivorous/Carnivorous
    return *this;
  }
  Aquarium& add_seaweed(size_t nb = 1) {
    for (size_t i = 0; i < nb; ++i) {
      m_world.entity().add<Seaweed>();
    }
    return *this;
  }

  void tick() {
    m_world.progress();
    println("There are {} seaweeds and {} fishes:", m_world.count<Seaweed>(), m_world.count<Fish>());
    m_world.each<Fish>([](flecs::entity e, Fish const& f) {
      cout << "\t- " << f.name << "(" << toChar(f.sex) << ") "
           << "(" << e.target<SpeciesR>().name() << ")\n";
    });
  }
private:
  flecs::world m_world;
  flecs::query<Seaweed> m_seaweedQ;
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

  for (size_t i = 0; i < 15; ++i) {
    println("----- Step {} -----", i);
    aq.tick();
  }
  return 0;
}
