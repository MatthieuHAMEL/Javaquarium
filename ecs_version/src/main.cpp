#include "flecs.h"
import std;
using namespace std;

enum class Sex { F, M };

struct Fish {
  std::string name;
  Sex sex;
};
struct Seaweed{};

class Aquarium
{
public:
  Aquarium() : m_world{} {
  }
  Aquarium& add_fish(std::string name, Sex sex) {
    println("Adding {}", name);
    m_world.entity(name.c_str()) // facilitates the queries
      .set<Fish>({std::move(name), sex});
    ++m_fishCount;
    return *this;
  }
  Aquarium& add_seaweed(size_t n = 1) {
    for (size_t k = 0; k < n; ++k) {
      println("Adding a seaweed");
      m_world.entity().add<Seaweed>();
      ++m_seaweedCount;
    }
    return *this;
  }

  void performTimestep() {
    println("-----");
    println("There are {} seaweeds and {} fishes:", m_seaweedCount, m_fishCount);

    m_world.each<Fish>([](Fish const& fish) {
      std::cout << " - " << fish.name << " ("
                << (fish.sex == Sex::M ? "M" : "F") << ")\n";
    });
  }
private:
  flecs::world m_world;
  size_t m_fishCount = 0;
  size_t m_seaweedCount = 0;
};

auto main() -> int {
  Aquarium aq;
  aq.add_fish("Bob", Sex::M)
    .add_fish("Brenda", Sex::F)
    .add_fish("Rhonda", Sex::F)
    .add_fish("Francesca", Sex::F)
    .add_fish("Gerard", Sex::M)
    .add_seaweed(10);

  for (size_t i = 0; i < 10; ++i) {
    aq.performTimestep();
  }
  return 0;
}
