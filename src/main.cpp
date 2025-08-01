#include "main.h"
#include "config.hpp"
#include "colorsort.hpp"
#include "autons.hpp"

#include <iostream>

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
    // Initialize hardware
    initializeHardware();
    
    // Initialize color sorting system
    initializeColorSorting();

    printf("Robot initialization complete\n");

    // Thread for brain screen and position logging
    pros::Task screenTask([&]() {
        while (true) {
            // Print robot location to the brain screen
            pros::lcd::print(0, "X: %f", chassis.getPose().x); // x
            pros::lcd::print(1, "Y: %f", chassis.getPose().y); // y
            pros::lcd::print(2, "Theta: %f", chassis.getPose().theta); // heading
            // Log position telemetry
            lemlib::telemetrySink()->info("Chassis pose: {}", chassis.getPose());
            // Delay to save resources
            pros::delay(50);
        }
    });
}

/**
 * Runs while the robot is disabled
 */
void disabled() {}

/**
 * Runs after initialize if the robot is connected to field control
 */
void competition_initialize() {
    
}

/**
 * Runs during autonomous period
 */
void autonomous() {
    
    //defaultAuton();
    deuxauto();
    // Alternative: use different autons 
    // redSideAuton();
    // blueSideAuton();
    // skillsAuton();
}

/**
 * Runs in driver control
 */
void opcontrol() {
    printf("Driver control started\n");
    
    // Loop forever
    while (true) {
        const int linear_power = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
        const int lateral_power = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
        chassis.arcade(linear_power, lateral_power);

        // Basket toggle 
        if (basket_toggle) {
            basket.set_value(true);    
        } else {
            basket.set_value(false);
        }
        
        // Intake controls with color sorting 
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
            setIntakeTop();
        }
        else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            setMidScoring();
        }
        else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
            setHighScoring();
        }
        else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
            setLowScoring();
        }
        else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_Y)){
            bottomrollers.move_velocity(600);      
            midrollers.move_velocity(600);         
            backroller.move_velocity(600);
        }
        else {
            setIdle();
        }
        
        // Color mode toggle
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_UP)) {
            toggleColorMode();
        }
        
        
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A)) {
            lower_toggle = !lower_toggle;
        }
        
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_X)) {
            basket_toggle = !basket_toggle;
        }
        
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN)) {
            scraper_toggle = !scraper_toggle;
        }
        
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B)){
            instapark.set_value(true);
        }
        
        
        if (scraper_toggle) {
            scraper.set_value(true);
        } else {
            scraper.set_value(false);
        }
        
        if (lower_toggle) {
            lower_basket.set_value(true);
        } else {
            lower_basket.set_value(false);
        }

        // Update color sorting system
        updateColorSorting();

        // Delay to save resources
        pros::delay(25);
    }
}
