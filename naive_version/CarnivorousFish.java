package javaquarium;

public abstract class CarnivorousFish extends BaseFish implements Carnivorous 
{
	public CarnivorousFish(String name, Gender gender) {
		super(name, gender);
	}

	@Override
	public void eat(BaseFish other) {
		System.out.printf("I am %s and I'm eating %s ... yummy.\n", this.getName(), other.getName());
		this.addOrRemoveHP(5);
		other.addOrRemoveHP(-4);
	}
}
