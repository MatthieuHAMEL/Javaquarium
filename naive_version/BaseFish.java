/**
 * @author Matthieu HAMEL
 */
package javaquarium;
/**
 * @param <T>
 * 
 */
public abstract class BaseFish extends LivingBeing {
	public enum Gender { Male, Female }
	
	public BaseFish(String name, Gender gender) {
		if (name == null) {
			throw new IllegalArgumentException("A fish must have a name");
		}
		this.name = name;
		this.gender = gender;
	}
	
	public String toString() {
		return this.name + ", " + this.gender.toString();
	}
	
	/** A fish is hungry and loses 1 HP per turn */
	public void performTimestep() {
		this.increaseAge();
		this.addOrRemoveHP(-1);
		
		if (this instanceof SexuallyHermaphroditic) {
			((SexuallyHermaphroditic) this).changeSex(this); // A bit clunky...
			// Note that sex may not change if the age is not half of the life expectancy!
		}
	}
	
	public boolean isHungry() {
		return (this.getHP() <= 5);
	}
	
	public String getName() { return this.name;	}
	public Gender getGender() { return this.gender;	}
	public void setGender(Gender g) {
		this.gender = g;
	}
	
	
	private String name;
	private Gender gender;
}


