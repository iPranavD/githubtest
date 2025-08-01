#include "autons.hpp"
#include "config.hpp"
#include "colorsort.hpp"

// Get path asset (put this outside functions)
ASSET(example_txt); // '.' replaced with "_" to make c++ happy
//yo
/**
 * Helper function to deploy intake and start color sorting
 */

/**
 * Red side autonomous routine
 */
void redSideAuton() {
    printf("Starting Red Side Autonomous\n");
    
    // Set color mode for red alliance
    currentColorMode = RED_MODE;
    
    // Start intake with color sorting
    setIntakeTop();
    
    // Example red side path
    chassis.moveToPose(10, 30, 90, 2000);
    chassis.turnToHeading(135, 2000);
    chassis.moveToPoint(39, 20, 3000);
    chassis.turnToHeading(180, 2000);
    
    // Use scraper directly
    scraper.set_value(true);
    
    chassis.moveToPoint(39, 0, 2000);
    pros::delay(2500);
    scraper.set_value(false);
    chassis.moveToPoint(39, 5, 2000);
    chassis.turnToHeading(0, 2000);
    
    printf("Red Side Autonomous Complete\n");
}

/**
 * Blue side autonomous routine
 */
void deuxauto() {
    printf("Starting Blue Side Autonomous\n");
    
    // Set color mode for red alliance (change as needed)
    currentColorMode = RED_MODE;
    
    // Start intake with color sorting
    setIntakeTop();
    chassis.moveToPose(0, 40, 0, 3000);
    chassis.moveToPose(-13, 40, -59, 3000);
    pros::delay(4000);
    /*chassis.turnToHeading(180, 2000);
    scraper.set_value(true);
    chassis.moveToPoint(39, 0, 2000);xddd
    pros::delay(2500);
    scraper.set_value(false);
    chassis.moveToPoint(39, 5, 2000);*/
    setLowScoring();
  
    
    printf("Default Autonomous Complete\n");
}

/**
 * Skills autonomous routine
 */
void skillsAuton() {
    printf("Starting Skills Autonomous\n");
    
    // Disable color sorting for skills (usually want all rings)
    currentColorMode = DISABLED;
    
    // Start intake
    setIntakeTop();
    
    // Skills routine - collect and score as many points as possible
    // This is a placeholder - customize based on your strategy
    
    for (int i = 0; i < 3; i++) {
        // Move to ring collection area
        chassis.moveToPoint(24, 24, 3000);
        pros::delay(2000); // Collect rings
        
        // Move to scoring area
        chassis.moveToPoint(0, 0, 3000);
        scoreBasket();
        
        pros::delay(500);
    }
    
    printf("Skills Autonomous Complete\n");
}

/**
 * Default autonomous routine (same as original)
 */
void defaultAuton() {
    printf("Starting Default Autonomous\n");
    
    // Set color mode for red alliance (change as needed)
    currentColorMode = RED_MODE;
    
    // Start intake with color sorting
    setIntakeTop();
    
    chassis.moveToPose(17, 29, 90, 3000);
    
    chassis.turnToHeading(135, 2000);
    chassis.moveToPoint(31, 15, 3000);
    /*chassis.turnToHeading(180, 2000);
    scraper.set_value(true);
    chassis.moveToPoint(39, 0, 2000);xddd
    pros::delay(2500);
    scraper.set_value(false);
    chassis.moveToPoint(39, 5, 2000);*/
    chassis.turnToHeading(0, 2000);
    chassis.moveToPoint(31, 28, 2000);
    setHighScoring();
  
    
    printf("Default Autonomous Complete\n");

}