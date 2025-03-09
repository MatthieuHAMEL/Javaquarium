package javaquarium;


public abstract class VegetarianFish extends BaseFish implements Vegetarian 
{
	public VegetarianFish(String name, Gender gender) {
		super(name, gender);
	}

	@Override
	public void eat(Seaweed sw) {
		System.out.printf("I am %s and I eat a seaweed ... yummy.\n", this.getName());
		this.addOrRemoveHP(3);
		sw.addOrRemoveHP(-2);
	}
}
