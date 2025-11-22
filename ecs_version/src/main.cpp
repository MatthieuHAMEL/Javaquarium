#include "flecs.h"
import std;
using namespace std;

enum class Sex { F, M };

struct Fish {
  std::string name;
  Sex sex;
};
struct Seaweed{};

// The fish species
struct SpeciesR{}; // Relationship
struct Bass{}; struct Tuna{}; struct ClownFish{}; struct Grouper{}; struct Sole{}; struct Carp{};

// The feeding behaviors
struct FeedingR{}; // Relationship
struct Herbivorous{}; struct Carnivorous{};

// Traits mapping specy -> behaviors
template<typename T> struct Characteristics{};
template<> struct Characteristics<Bass> { using feeding = Carnivorous; };
template<> struct Characteristics<Tuna> { using feeding = Carnivorous; };
template<> struct Characteristics<ClownFish> { using feeding = Carnivorous; };
template<> struct Characteristics<Grouper> { using feeding = Herbivorous; };
template<> struct Characteristics<Sole> { using feeding = Herbivorous; };
template<> struct Characteristics<Carp> { using feeding = Herbivorous; };

class Aquarium
{
public:
  template<typename SPECY>
  Aquarium& add_fish(std::string name, Sex sex) {
    using Feeding = typename Characteristics<SPECY>::feeding;
    
    m_world.entity(name.c_str()).set<Fish>({std::move(name), sex})
      .add<SpeciesR, SPECY>() // Tuna, Bass, ClownFish...
      .template add<FeedingR, Feeding>(); // Herbivorous/Carnivorous
    ++m_fishCount;
    return *this;
  }
  Aquarium& add_seaweed(size_t n = 1) {
    for (size_t k = 0; k < n; ++k) {
      m_world.entity().add<Seaweed>();
      ++m_seaweedCount;
    }
    return *this;
  }

  void tick() {
    println("There are {} seaweeds and {} fishes:", m_seaweedCount, m_fishCount);
    m_world.each<Fish>([](flecs::entity e, Fish const& f) {
      cout << "\t- " << f.name << "(" << (f.sex == Sex::M ? "M" : "F") << ") "
           << "(" << e.target<SpeciesR>().name() << ")\n";
    });
  }
private:
  flecs::world m_world;
  size_t m_fishCount = 0; // The counters are not strictly necessary, I could use queries
  size_t m_seaweedCount = 0;
};

auto main() -> int {
  Aquarium aq;
  aq.add_fish<Bass>("Bob", Sex::M)
    .add_fish<Tuna>("Brenda", Sex::F)
    .add_fish<Grouper>("Rhonda", Sex::F)
    .add_fish<ClownFish>("Francesca", Sex::F)
    .add_fish<Tuna>("Gerard", Sex::M)
    .add_seaweed(10);

  for (size_t i = 0; i < 10; ++i) {
    println("----- Step {} -----", i);
    aq.tick();
  }
  return 0;
}
