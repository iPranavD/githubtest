#include "main.h"
#include "lemlib/api.hpp"

 // Controller
pros::Controller controller(pros::E_CONTROLLER_MASTER);

// Drivetrain motors
pros::MotorGroup leftMotors({11, -12, -13}, pros::MotorGearset::blue);
pros::MotorGroup rightMotors({-14, 15, 16}, pros::MotorGearset::blue);

// Indexing/intake motors
pros::Motor toprollers(-10, pros::MotorGearset::blue);
pros::Motor midrollers(-13, pros::MotorGearset::blue);
pros::Motor bottomrollers(17, pros::MotorGearset::blue);

// Sensors

//pros::Imu imu(21);
pros::adi::DigitalOut scraper('A');
pros::adi::DigitalOut mid_scoring('D');


//tracking wheels
pros::Rotation horizontalEnc(-1);
// vertical tracking wheel encoder. Rotation sensor, port 1, reversed
pros::Rotation verticalEnc(20);
pros::Imu imu(21);
lemlib::TrackingWheel horizontal(&horizontalEnc, lemlib::Omniwheel::NEW_2, -5.75);
// vertical tracking wheel. 2.75" diameter, 2.5" offset, left of the robot (negative)
lemlib::TrackingWheel vertical(&verticalEnc, lemlib::Omniwheel::NEW_2, -2.5);

// Drivetrain settings
lemlib::Drivetrain drivetrain(&leftMotors, // left motor group
                              &rightMotors, // right motor group
                              6, // 10.5 inch track width
                              lemlib::Omniwheel::NEW_4, // using new 3.25" omnis
                              343, // drivetrain rpm is 343
                              2 // horizontal drift is 2. If we had traction wheels, it would have been 8
);

// Lateral motion controller
lemlib::ControllerSettings linearController(9, // proportional gain (kP)
                                            0, // integral gain (kI)
                                            9, // derivative gain (kD)
                                            3, // anti windup
                                            1, // small error range, in inches
                                            100, // small error range timeout, in milliseconds
                                            3, // large error range, in inches
                                            500, // large error range timeout, in milliseconds
                                            60 // maximum acceleration (slew)
);

// Angular motion controller
lemlib::ControllerSettings angularController(4, // proportional gain (kP)
                                             0, // integral gain (kI)
                                             28, // derivative gain (kD)
                                             30, // anti windup
                                             10, // small error range, in degrees
                                             100, // small error range timeout, in milliseconds
                                             30, // large error range, in degrees
                                             500, // large error range timeout, in milliseconds
                                             60 // maximum acceleration (slew)
);

// Sensors for odometry
lemlib::OdomSensors sensors(nullptr, // vertical tracking wheel
                            nullptr, // vertical tracking wheel 2, set to nullptr as we don't have a second one
                            nullptr, // horizontal tracking wheel
                            nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                            &imu// inertial sensor
);

// Input curve for throttle input during driver control
lemlib::ExpoDriveCurve throttleCurve(3, // joystick deadband out of 127
                                     10, // minimum output where drivetrain will move out of 127
                                     1.019 // expo curve gain
);

// Input curve for steer input during driver control
lemlib::ExpoDriveCurve steerCurve(3, // joystick deadband out of 127
                                  10, // minimum output where drivetrain will move out of 127
                                  1.019 // expo curve gain
);

// Create the chassis
lemlib::Chassis chassis(drivetrain, linearController, angularController, sensors, &throttleCurve, &steerCurve);

// Position source enum

// Turn to heading function

// Initialize all hardware components
void initializeHardware() {
    pros::lcd::initialize(); // initialize brain screen
    chassis.calibrate(); // calibrate sensors
    // Initialize optical sensors
    printf("Hardware initialization complete\n");
}
void setIntakeTop() {
    bottomrollers.move_velocity(600);
    toprollers.move_velocity(0);
}

void setMidScoring() {
    bottomrollers.move_velocity(600);
    toprollers.move_velocity(400);
}

void setHighScoring() {
    bottomrollers.move_velocity(600);
    toprollers.move_velocity(600);
}

void setLowScoring() {
    bottomrollers.move_velocity(-500);
    toprollers.move_velocity(-500);
}

void setIdle() {
    bottomrollers.move_velocity(0);
    toprollers.move_velocity(0);
    mid_scoring.set_value(true);

}
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
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */

 void right5() {
    setIntakeTop();
    chassis.moveToPoint(5.5, 38, 2000, {.maxSpeed = 40});
    chassis.waitUntilDone();
    // chassis.moveToPose(-28, 48, -65, 2000, {.maxSpeed = 40});
    // chassis.waitUntilDone();
    // scraper.set_value(true);
    // pros::delay(500);
    // chassis.moveToPoint(-5, 31, 2000, {.forwards = false});
    chassis.waitUntilDone();
    chassis.turnToHeading(135, 2000);
    chassis.moveToPoint(36, 9, 2000);
    scraper.set_value(false);
    chassis.turnToHeading(180, 2000);
    chassis.moveToPoint(36, 34, 2000, {.forwards = false, .maxSpeed = 70});
    chassis.waitUntilDone();
    setHighScoring();
    mid_scoring.set_value(false);
    pros::delay(3260);
    setIntakeTop();
    scraper.set_value(true);
    chassis.moveToPoint(36, -7, 2000, {.maxSpeed = 600});
    chassis.waitUntilDone();
    pros::delay(150);
    scraper.set_value(false);
    chassis.moveToPoint(36, 34, 2000, {.forwards = false, .maxSpeed = 60});
    chassis.waitUntilDone();
    setHighScoring();
}

void left5(){
    setIntakeTop();
    chassis.moveToPoint(-5.2, 38, 2000, {.maxSpeed = 40});
    chassis.waitUntilDone();
    // chassis.moveToPose(-28, 48, -65, 2000, {.maxSpeed = 40});
    // chassis.waitUntilDone();
    // scraper.set_value(true);
    // pros::delay(500);
    // chassis.moveToPoint(-5, 31, 2000, {.forwards = false});
    chassis.waitUntilDone();
    chassis.turnToHeading(-135, 2000);
    chassis.moveToPoint(-36, 9, 2000);
    scraper.set_value(false);
    chassis.turnToHeading(180, 2000);
    chassis.moveToPoint(-36, 34, 2000, {.forwards = false, .maxSpeed = 70});
    chassis.waitUntilDone();
    setHighScoring();
    mid_scoring.set_value(false);
    pros::delay(3260);
    setIntakeTop();
    scraper.set_value(true);
    chassis.moveToPoint(-36, -7, 2000, {.maxSpeed = 360});
    chassis.waitUntilDone();
    pros::delay(150);
    scraper.set_value(false);
    chassis.moveToPoint(-36, 34, 2000, {.forwards = false, .maxSpeed = 60});
    chassis.waitUntilDone();
    setHighScoring();
}

void skills(){
    chassis.setPose(-58.387,-14.844, 90);
    setIntakeTop();
    chassis.moveToPoint(-22.761, -23.157, 6000, {.maxSpeed = 40});
    chassis.waitUntilDone();
    pros::delay(50);
    chassis.moveToPoint(-0.198, -31.866, 2500);
    chassis.waitUntilDone();
    setIdle();
    chassis.turnToHeading(180, 1000);
    chassis.waitUntilDone();
    chassis.moveToPoint(0, -37.793, 2000, {.maxSpeed = 40});
    chassis.waitUntilDone();
    setIntakeTop();
    scraper.set_value(true);
    pros::delay(750);
    chassis.turnToPoint(-29.491, -29.491, 750);
    chassis.waitUntilDone();
    chassis.moveToPoint(-29.491, -29.491, 1000);
    pros::delay(200);
    scraper.set_value(false);
    chassis.waitUntilDone();
    chassis.turnToPoint(-42.251, -49.502, 750);
    chassis.waitUntilDone();
    chassis.moveToPoint(-42.251, -51.502, 1000);
    chassis.waitUntilDone();
    chassis.turnToHeading(270, 1000);
    chassis.waitUntilDone();
    chassis.moveToPoint(-29.584, -48.502, 1000, {.forwards = false});
    chassis.waitUntilDone();
    setHighScoring();
    pros::delay(4000);
    scraper.set_value(true);
    chassis.moveToPoint(-30.012, -49.085, 1000);
    chassis.waitUntilDone();
    pros::delay(200);
    setIntakeTop();
    pros::delay(4000);
    chassis.moveToPoint(-29.615, -47.106, 1000, {.forwards = false});
    chassis.waitUntilDone();
    pros::delay(200);
    setHighScoring();
    pros::delay(4000);
    chassis.moveToPoint(-39.282, -30.282, 1000);
    chassis.waitUntilDone();

}
void skills2(){
    chassis.setPose(0, 0, 0);
    setIntakeTop();
    chassis.moveToPoint(6, 40, 2000, {.maxSpeed = 40});
    chassis.waitUntilDone();
    chassis.turnToHeading(125, 2000);
    chassis.moveToPoint(37, 10, 2000);
    chassis.turnToHeading(180, 2000);
    chassis.moveToPoint(36.7, 34, 2000, {.forwards = false, .maxSpeed = 100});
    chassis.waitUntilDone();
    setHighScoring();
    mid_scoring.set_value(false);
    setLowScoring();
    pros::delay(100);
    setHighScoring();
    pros::delay(3300);
    scraper.set_value(true);
    setIntakeTop();
    chassis.moveToPoint(36, -8, 2000, {.maxSpeed = 999});
    chassis.waitUntilDone();
    pros::delay(3000);
    chassis.moveToPoint(36.7, 34, 2000, {.forwards = false, .maxSpeed = 100});
    chassis.waitUntilDone();
    setHighScoring();
    setLowScoring();
    pros::delay(300);
    setHighScoring();
    pros::delay(3600);
    scraper.set_value(false);
    chassis.moveToPoint(37.5, 19, 1000);
    chassis.turnToHeading(-90, 1000);
    setIntakeTop();
    chassis.moveToPoint(-40, 23, 2000);
    chassis.waitUntilDone();
    chassis.moveToPoint(-63, 5, 2000);
    chassis.turnToHeading(180, 1000);
    chassis.moveToPoint(-63, 34, 2000, {.forwards = false});
    chassis.waitUntilDone();
    setHighScoring();
    pros::delay(3300);
    scraper.set_value(true);
    setIntakeTop();
    chassis.moveToPoint(-62, -7.5, 2000, {.maxSpeed = 999});
    chassis.waitUntilDone();
    pros::delay(1000);
    chassis.moveToPoint(-63, 34, 2000, {.forwards = false, .maxSpeed = 70});
    chassis.waitUntilDone();
    setHighScoring();
    pros::delay(3300);
    scraper.set_value(false);
    chassis.moveToPoint(-9.25, 19, 2500, {.maxSpeed = 70});
    chassis.turnToHeading(180, 2000);
    chassis.moveToPoint(-9.25, -8, 2000);
    setLowScoring();
}
    

void autonomous() {
    //right5();
    //skills2();
    left5();
}



/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
    printf("Driver control started\n");
    
    // Loop forever
    while (true) {
        bool park_toggle = false;
        static bool scraper_toggle {false};
        bool hood_toggle = false;
        const int lateral_power = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y); 
        const int linear_power = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X); 
        chassis.arcade(lateral_power, linear_power);

        // Basket toggle 
        
        // Intake controls with color sorting 
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
            setIntakeTop();
                                    mid_scoring.set_value(true);
        }
        else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            setMidScoring();
                                    mid_scoring.set_value(true);
        }
        else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
            setHighScoring();
                    mid_scoring.set_value(false);
        }
        else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) { 
            setLowScoring();
                                    mid_scoring.set_value(true);
        }
        else {
            setIdle();
                                    mid_scoring.set_value(true);
        }
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN)){
            scraper_toggle = !scraper_toggle;
        }
        if (scraper_toggle){
            scraper.set_value(true);
        }
        else if (!scraper_toggle) {
            scraper.set_value(false);
        }
    
        // Color mode toggle
        // Delay to save resources
        pros::delay(25);
    }
}