/**
 * @author Matthieu HAMEL
 */
package javaquarium;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import javaquarium.BaseFish.Gender;

/**
 * 
 */
public class Aquarium 
{	
	Random randGen = new Random();
	
	public int getNbSeaweeds() {
		return this.seaweeds.size();
	}
	
	public int getNbFishes() {
		return this.fishes.size();
	}
	
	public void addNewSeaweed() {
		this.seaweeds.add(new Seaweed());
	}
	
	public void addSeaweed(Seaweed sw) {
		this.seaweeds.add(sw);
	}
	
	/** 
	 * Precondition: all seaweeds in the array are alive. 
	 * @return false if there were no available seaweed */
	private int randomSeaweed() {
		// Choose a random seaweed, if there is one
		if (this.getNbSeaweeds() == 0) {
			return -1;
		}
		return randGen.nextInt(getNbSeaweeds());
	}
	
	/** @return the index of the picked fish, -1 if there is no applicable fish 
	 * Some fishes in the array may already be dead, this is took into account.*/
	private int randomFishIdxDifferentFrom(BaseFish fish, boolean toEat) { // TODO something cleaner.
		BaseFish res;
		int index = -2; // -2 is an unexpected retval here ...
		do 
		{
			index = randGen.nextInt(getNbFishes());
			res = fishes.get(index);
			if (res == fish) {
				return -1; // Fish has just 1 try : if it tries to eat itself or a same species fish, it's over
			} 
			if (toEat && res.getClass().equals(fish.getClass())) { // For predator fishes (we must pick a different species)
				return -1;
			}
			else if (!toEat && !res.getClass().equals(fish.getClass())) { // For lover fishes (we must pick the same species)
				return -1;
			}
		} while (res.isDead()); // But eating a dead fish doesn't count (leaving dead fishes in the aquarium for the rest 
								// of the turn is implementation detail so we try again).
		// The loop will terminate since even if all other fishes are dead or of the same species, our current fish will 
		// try to eat itself at some point.
		return index;
	}
	
	public void addGrouper(String name, Gender gender) {
		this.fishes.add(new Grouper(name, gender));
	}
	public void addTuna(String name, Gender gender) {
		this.fishes.add(new Tuna(name, gender));
	}
	public void addClownfish(String name, Gender gender) {
		this.fishes.add(new Clownfish(name, gender));
	}
	public void addSole(String name, Gender gender) {
		this.fishes.add(new Sole(name, gender));
	}
	public void addBass(String name, Gender gender) {
		this.fishes.add(new Bass(name, gender));
	}
	public void addCarp(String name, Gender gender) {
		this.fishes.add(new Carp(name, gender));
	}
	public void addFish(BaseFish fish) {
		this.fishes.add(fish);
	}
	
	/** Simulate time passing in the aquarium
	 * For now advantage is clearly given to the first carnivorous in the list */
	public void performTimestep() {
		// Let seaweeds and fishes evolve 
		List<Seaweed> babies_sw = new ArrayList<Seaweed>();
		for (Seaweed sw: seaweeds) {
			sw.performTimestep();
			Seaweed baby_sw = sw.reproduceIfNeeded();
			if (baby_sw != null) {
				babies_sw.add(baby_sw);
			}
		}
		// Remove dead seaweeds and add baby seaweeds
		seaweeds.removeIf(sw -> (sw.isDead()));
		for (Seaweed sw : babies_sw) {
			this.addSeaweed(sw);
		}
		
		List<BaseFish> babies = new ArrayList<BaseFish>();
		for (BaseFish fish: this.fishes) {
			fish.performTimestep();
			
			// "Fishes that are not hungry will just go have a drink with another fish
			// If that other fish happens to be of the same species and of the opposite sex, 
			// they give birth to a baby fish, that has the same species and a random gender".
			if (!fish.isHungry()) {
				int idx = randomFishIdxDifferentFrom(fish, false);
				// If not -1, idx designates a same-species, living fish, that our current fish could go for a drink with.
				// We must ensure that it is an opposite sex fish 
				if (idx != -1) 
				{
					BaseFish lover = this.fishes.get(idx);
					if (lover.getGender() == fish.getGender()) { // TODO enforce LGBTQ+ rights in the aquarium
						System.out.printf("%s and %s are dating!\n", fish.getName(), lover.getName()); 
						// Let a chance for 'fish' to change its sex to have a baby with lover
						// Yes fishes can do that ...
						if (fish instanceof SexuallyOpportunistic) {
							((SexuallyOpportunistic) fish).changeSex(fish);
						}
					}	
					
					// Now its still possible that they have the same sex if fish wasn't opportunistic, so test it :
					if (lover.getGender() != fish.getGender()) {
						// Fishes can give birth to a baby fish
						String babyName = fish.getName().substring(0, Math.min(3, fish.getName().length()))
								+ lover.getName().substring(0, Math.min(3, lover.getName().length()));
						try {
							// Obviously ugly -- see my README!
							BaseFish baby = fish.getClass().getConstructor(String.class, Gender.class).newInstance(babyName, Math.random() < 0.5 ? Gender.Male : Gender.Female);
							babies.add(baby);
							System.out.printf("A new fish was born : %s\n", baby);
						} catch (Exception e) {
					        e.printStackTrace();
						}
					}
				}
				continue; // Fish is not hungry, they won't try to eat anything
			}
			
			// At this point our fish is hungry. Let's eat!
			if (fish instanceof Vegetarian) {
				int idx = randomSeaweed();
				if (idx == -1) {
					System.out.println("No more seaweed to eat!");
					continue;
				}
				((Vegetarian) fish).eat(this.seaweeds.get(idx));
				if (this.seaweeds.get(idx).isDead()) {
					// Remove the seaweed if it's dead 
					// (I don't do the same with fishes since I'm iterating on them - it's done after the loop)
					this.seaweeds.remove(idx);
				}
			}
			else { // Carnivorous fish
				int idx = randomFishIdxDifferentFrom(fish, true);
				if (idx == -1) {
					System.out.printf("%s failed to eat a fish\n", fish.getName());
					continue;
				}
				BaseFish victim = this.fishes.get(idx);
				((CarnivorousFish) fish).eat(victim);
			}
		}
		
		// Once the iteration is done, remove the fishes marked as dead.
		fishes.removeIf(fish -> 
			{
				if (fish.isDead()) {
					System.out.printf("%s passed away at age %d ... !\n", fish.getName(), fish.getAge());
				}
				return fish.isDead();
			});
		
		// Add babies
		for (BaseFish b : babies) {
			this.addFish(b);
		}
		
		System.out.printf("- There are %d seaweed(s)\n- There are %d fishe(s)\n", this.getNbSeaweeds(), this.getNbFishes());
		for (BaseFish fish: this.fishes) {
			System.out.printf("\t- %s (%d HP)\n", fish, fish.getHP());
		}
		System.out.println("------- End of turn!\n\n");
	}
	
	private ArrayList<Seaweed> seaweeds = new ArrayList<Seaweed>(); 
	private ArrayList<BaseFish> fishes = new ArrayList<BaseFish>();
}
