package javaquarium;

import javaquarium.BaseFish.Gender;

public class Main {

	public static void main(String[] args) 
	{
		var myAquarium = new Aquarium();
		
		for (int i = 0; i < 40; i++) {
			myAquarium.addNewSeaweed();
		}
		myAquarium.addGrouper("Cleopatra", Gender.Female);
		myAquarium.addBass("Babar", Gender.Male);
		myAquarium.addClownfish("Matou", Gender.Male);
		myAquarium.addTuna("Casimir", Gender.Male);
		myAquarium.addBass("Hippolyte", Gender.Male);
		myAquarium.addCarp("Naruto", Gender.Male);
		myAquarium.addCarp("Dory", Gender.Female);
		myAquarium.addSole("Francine", Gender.Female);
		myAquarium.addClownfish("Nemo", Gender.Male);
		for (int i = 0; i < 10; i++) {
			myAquarium.performTimestep();
		}
	}

}
