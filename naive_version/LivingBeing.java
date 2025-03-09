package javaquarium;

public abstract class LivingBeing {
	private static final int LIFE_EXPECTANCY = 20;
	
	public LivingBeing() {
		this.HP = 10;
		this.age = 0;
	}

	/**
	 * @return the health points
	 */
	public int getHP() {
		return HP;
	}
	public int getAge() {
		return age;
	}
	
	public void increaseAge() {
		age += 1;
	}
	
	public void addOrRemoveHP(int nbHP) {
		this.HP += nbHP;
	}
	
	public boolean isDead() { // Makes use of getLifeExpectancy virtuality ...
		return (getHP() <= 0 || getAge() == getLifeExpectancy());
	}
	
	public int getLifeExpectancy() { // Can be overriden by fishes or seaweeds if needed
		return LivingBeing.LIFE_EXPECTANCY;
	}

	private int HP;
	private int age;
}
