# Javaquarium
A reference OOP exercise by SpaceFox (https://zestedesavoir.com/forums/sujet/447/javaquarium/)

The goal of this exercise is to show the difficulties of "simplistic" OOP - especially when it looks like multiple inheritance would be the solution (and since it doesn't exist in Java, there are ways to bypass ... but things get more and more counter-intuitive). 

Basically we have fishes in an aquarium (a container). There are several species of fishes : Grouper, Carp, Bass, Clownfish ... they share behaviors like having a name or a gender, and healthpoints, but some of them are vegetarian and eat seaweeds, some of them are carnivorous. There are different sexual behaviors also which are specific to each fish type.

I voluntarily developped a "naive" approach in the naive_version folder : with a big inheritance tree and a bunch of interfaces. I get into problems when I have to test if two fishes are of the same species, or when I must construct a new fish from another one's type. This version falls into the trap of using ```instanceof``` and reflection (getClass()) everywhere (alternatively enumerated types mapping the inheritance tree, which I didn't do).

My goal is to provide another version in the future, using a compositional / component-based philosophy (ECS).
