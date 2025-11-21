#include "flecs.h"
import std;
using namespace std;

enum class Sex { F, M };

struct Fish {
  std::string name;
  Sex s;
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
    return *this;
  }
  Aquarium& add_seaweed(size_t n = 1) {
    for (size_t k = 0; k < n; ++k) {
      println("Adding a seaweed");
      m_world.entity().add<Seaweed>();
    }
    return *this;
  }

  
private:
  flecs::world m_world;
};

auto main() -> int {
  Aquarium aq;
  aq.add_fish("Bob", Sex::M)
    .add_fish("Brenda", Sex::F)
    .add_fish("Rhonda", Sex::F)
    .add_fish("Francesca", Sex::F)
    .add_fish("Gerard", Sex::M)
    .add_seaweed(5);
  return 0;
}
