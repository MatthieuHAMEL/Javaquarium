package javaquarium;

public class Seaweed extends LivingBeing {

	public Seaweed() {
		// TODO Auto-generated constructor stub
		super();
	}
	
	/** A seaweed grows every turn (it gains 1 HP) */
	public void performTimestep() {
		this.increaseAge();
		this.addOrRemoveHP(1);
	}
	
	public Seaweed reproduceIfNeeded() {
		if (this.getHP() >= 10) {
			Seaweed sw = new Seaweed();
			sw.addOrRemoveHP(-(sw.getHP() / 2));
			this.addOrRemoveHP(-(this.getHP() / 2));
			return sw;
		}
		return null;
	}

}
