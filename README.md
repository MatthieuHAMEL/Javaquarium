# Javaquarium
A reference OOP exercise by SpaceFox (https://zestedesavoir.com/forums/sujet/447/javaquarium/)

The goal of this exercise is to show the dangers of "naive" OOP - especially when multiple orthogonal behavioral axes are needed, and when it looks like multiple inheritance would be the solution.

Basically we have fishes in an aquarium (a container). There are several species of fishes : Grouper, Carp, Bass, Clownfish ... they share behaviors like having a name or a gender, and healthpoints, but some of them are vegetarian and eat seaweeds, some of them are carnivorous. There are different sexual behaviors also which are specific to each fish type.

I voluntarily developped a "naive" approach in the naive_version folder : with a big inheritance tree and a bunch of interfaces. I get into problems when I have to test if two fishes are of the same species, or when I must construct a new fish from another one's type. This version falls into the trap of using ```instanceof``` and reflection (getClass()) everywhere (another alternative being having enumerated types values mapping the inheritance tree and a bunch of switch/cases, which I didn't do).

A cleaner version in Java would use more composition and behaviors ("strategy pattern") : consider that ugly code:

```java
// Aquarium.java
// I must call fish.eat() but I don't know which eat() method, I need to check the fish type:
if (fish instanceof Vegetarian) {
  // ...
  ((Vegetarian) fish).eat(this.seaweeds.get(idx));
}
else { // Carnivorous fish
  // ...
  BaseFish victim = this.fishes.get(idx);
  ((CarnivorousFish) fish).eat(victim);
}
```

Instead of relying on ```instanceof``` and having that logic in the aquarium, we can rely on a FeedingBehavior stored as a BaseFish attribute. Then the logic is implemented in the behaviors and is kept generic in the rest of the code (i.e the Aquarium) :

```
public abstract class BaseFish extends LivingBeing {
  private final FeedingBehavior feedingBehavior; // same for "SexualBehavior"!
  //...
  public BaseFish(String name, Gender gender, FeedingBehavior feedingBehavior) {
        this.feedingBehavior = feedingBehavior;
        //...
  }

  public void eat(Aquarium env) { // the aquarium just calls fish.eat(this)
    if (isHungry()) {
      feedingBehavior.eat(this, env); // dispatches on CarnivorousFeeding.eat or VegetarianFeeding.eat.
    }
  }
  //...
}

class Sole extends BaseFish { // No more inheritance vs interfaces mess, no more instanceof 
  public Sole(String name, Gender gender) {
    super(name, gender, new HerbivorousFeeding());
  }
}
```

Though I believe this comes with an expressivity cost: instead of one big inheritance/interface tree, there are now several parallel trees (one for each behavior) with functions capturing the BaseFish & Aquarium instances and mutating them from the outside ; at a large scale this can still be painful to read.

My goal is to provide another version in the future, using a component-based philosophy (ECS).
