package javaquarium;

import javaquarium.BaseFish.Gender;

public interface SexuallyOpportunistic {
	/** Contract : this should only be called if the fish has met some fish 
	 * from the same sex and wants to change its sex to reproduce with them.
	 */
	default void changeSex(BaseFish fish) {	
		// Reverse the gender 
		fish.setGender(fish.getGender() == Gender.Male ? Gender.Female : Gender.Male);
		System.out.printf("%s has changed its sex opportunistically !\n", fish.getName());
	}

}
