package javaquarium;

import javaquarium.BaseFish.Gender;

public interface SexuallyHermaphroditic {
	default void changeSex(BaseFish fish) {
        if (fish.getAge() == fish.getLifeExpectancy() / 2) {
        	System.out.printf("%s, hermaphroditic, has changed its sex !\n", fish.getName());
            fish.setGender(fish.getGender() == Gender.Male ? Gender.Female : Gender.Male); 
        }
    }

}
