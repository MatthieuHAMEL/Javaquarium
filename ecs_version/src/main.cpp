#include "flecs.h"
import std;

using namespace std;

auto main() -> int {
  flecs::world world;

  auto someEntity = world.entity();
  println("someEntity is alive ? -> {}", someEntity.is_alive());

  return 0;
}
