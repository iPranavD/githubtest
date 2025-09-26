#include "main.h"
#include "lemlib/api.hpp"

// Controller
pros::Controller controller(pros::E_CONTROLLER_MASTER);

// Dedicated drivetrain motors (always drivetrain - 6 motors)
pros::MotorGroup leftMotors({11, -12, -13}, pros::MotorGearset::blue);
pros::MotorGroup rightMotors({-14, 15, 16}, pros::MotorGearset::blue);

// PTO motors (switch between intake and drivetrain - 2 motors)
// These are your ONLY intake motors when disengaged
pros::Motor pto_left(-17, pros::MotorGearset::blue);   // PTO motor for left side
pros::Motor pto_right(18, pros::MotorGearset::blue);   // PTO motor for right side

// Pneumatics
pros::adi::DigitalOut scraper('A');
pros::adi::DigitalOut pto_piston('B'); // PTO control pneumatic

// PTO state tracking
bool pto_engaged = true;  // true = 8-motor drivetrain, false = 6-motor drive + 2 intake
bool last_pto_button = false;

// Sensors and tracking wheels (unchanged)
pros::Rotation horizontalEnc(-1);
pros::Rotation verticalEnc(20);
pros::Imu imu(17);
lemlib::TrackingWheel horizontal(&horizontalEnc, lemlib::Omniwheel::NEW_2, -5.75);
lemlib::TrackingWheel vertical(&verticalEnc, lemlib::Omniwheel::NEW_2, -2.5);

// Drivetrain settings - LemLib uses the 6 dedicated motors
lemlib::Drivetrain drivetrain(&leftMotors,
                              &rightMotors,
                              6,
                              lemlib::Omniwheel::NEW_4,
                              343,
                              2
);

// Controllers (unchanged)
lemlib::ControllerSettings linearController(9, 0, 9, 3, 1, 100, 3, 500, 60);
lemlib::ControllerSettings angularController(4, 0, 28, 30, 10, 100, 30, 500, 60);
lemlib::OdomSensors sensors(nullptr, nullptr, nullptr, nullptr, &imu);
lemlib::ExpoDriveCurve throttleCurve(3, 10, 1.019);
lemlib::ExpoDriveCurve steerCurve(3, 10, 1.019);
lemlib::Chassis chassis(drivetrain, linearController, angularController, sensors, &throttleCurve, &steerCurve);

// Initialize all hardware components
void initializeHardware() {
    pros::lcd::initialize();
    chassis.calibrate();
    
    // Initialize PTO to drivetrain mode (8-motor drive)
    pto_piston.set_value(true);  // Adjust based on your pneumatic setup
    pto_engaged = true;
    
    // Set brake modes
    pto_left.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);  // Coast for drivetrain
    pto_right.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
    
    printf("Hardware initialization complete\n");
}

// PTO Control Function
void handlePTO() {
    bool pto_button = controller.get_digital(pros::E_CONTROLLER_DIGITAL_X);
    
    // Check for button press (rising edge detection)
    if (pto_button && !last_pto_button) {
        // Toggle PTO state
        pto_engaged = !pto_engaged;
        
        // Control pneumatic based on PTO state
        pto_piston.set_value(pto_engaged);
        
        // Stop PTO motors during transition
        pto_left.move(0);
        pto_right.move(0);
        
        // Small delay for pneumatic to actuate
        pros::delay(150);
        
        // Update brake modes based on new state
        if (pto_engaged) {
            // Drivetrain mode - coast for smooth driving
            pto_left.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
            pto_right.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
        } else {
            // Intake mode - hold for precision control
            pto_left.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
            pto_right.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
        }
    }
    
    last_pto_button = pto_button;
}

// Modified intake functions - PTO motors act as intake when disengaged
void setIntakeTop() {
    bottomrollers.move_velocity(600);
    toprollers.move_velocity(0);
    if (!pto_engaged) {
        // Use PTO motors as intake motors
        pto_left.move_velocity(400);   // Mid-left intake roller
        pto_right.move_velocity(400);  // Mid-right intake roller
    }
}

void setMidScoring() {
    bottomrollers.move_velocity(600);
    toprollers.move_velocity(400);
    if (!pto_engaged) {
        pto_left.move_velocity(500);
        pto_right.move_velocity(500);
    }
}

void setHighScoring() {
    bottomrollers.move_velocity(600);
    toprollers.move_velocity(600);
    if (!pto_engaged) {
        pto_left.move_velocity(600);
        pto_right.move_velocity(600);
    }
}

void setLowScoring() {
    bottomrollers.move_velocity(-500);
    toprollers.move_velocity(-500);
    if (!pto_engaged) {
        pto_left.move_velocity(-500);
        pto_right.move_velocity(-500);
    }
}

void setIdle() {
    bottomrollers.move_velocity(0);
    toprollers.move_velocity(0);
    if (!pto_engaged) {
        pto_left.move_velocity(0);
        pto_right.move_velocity(0);
    }
}

// Custom drive function that adds PTO motors when engaged
void customDrive(int lateral, int linear) {
    // Always use the 6 dedicated drivetrain motors via LemLib
    chassis.arcade(lateral, linear);
    
    if (pto_engaged) {
        // Add the 2 PTO motors to drivetrain (8-motor total)
        int leftPower = lateral + linear;
        int rightPower = lateral - linear;
        
        // Apply the same power scaling as the main drivetrain
        // You may need to adjust these values based on your setup
        pto_left.move(leftPower);
        pto_right.move(rightPower);
    }
    // If PTO is disengaged, PTO motors are controlled by intake functions
}

void initialize() {
    initializeHardware();
    printf("Robot initialization complete\n");

    pros::Task screenTask([&]() {
        while (true) {
            pros::lcd::print(0, "X: %f", chassis.getPose().x);
            pros::lcd::print(1, "Y: %f", chassis.getPose().y);
            pros::lcd::print(2, "Theta: %f", chassis.getPose().theta);
            if (pto_engaged) {
                pros::lcd::print(3, "PTO: 8M DRIVE");
            } else {
                pros::lcd::print(3, "PTO: 6M + INTAKE");
            }
            lemlib::telemetrySink()->info("Chassis pose: {}", chassis.getPose());
            pros::delay(50);
        }
    });
}

void disabled() {}
void competition_initialize() {}

// Autonomous functions - work with either 6 or 8 motor drive
void right5() {
    setIntakeTop();
    chassis.moveToPoint(5.5, 38, 2000, {.maxSpeed = 40});
    chassis.waitUntilDone();
    chassis.turnToHeading(135, 2000);
    chassis.moveToPoint(36, 9, 2000);
    scraper.set_value(false);
    chassis.turnToHeading(180, 2000);
    chassis.moveToPoint(36, 34, 2000, {.forwards = false, .maxSpeed = 70});
    chassis.waitUntilDone();
    setHighScoring();
    pros::delay(3760);
    chassis.moveToPoint(36, 28, 3000);
    chassis.moveToPoint(36, 34, 2000, {.forwards = false, .maxSpeed = 140});
}

void left5(){
    setIntakeTop();
    chassis.moveToPoint(-5.2, 38, 2000, {.maxSpeed = 40});
    chassis.waitUntilDone();
    chassis.turnToHeading(-135, 2000);
    chassis.moveToPoint(-36, 9, 2000);
    scraper.set_value(false);
    chassis.turnToHeading(180, 2000);
    chassis.moveToPoint(-36, 34, 2000, {.forwards = false, .maxSpeed = 70});
    chassis.waitUntilDone();
    setHighScoring();
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

void autonomous() {
    right5();
}

void opcontrol() {
    printf("Driver control started\n");
    
    while (true) {
        static bool scraper_toggle {false};
        
        // Handle PTO switching
        handlePTO();
        
        // Get controller inputs
        const int lateral_power = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y); 
        const int linear_power = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X); 
        
        // Use custom drive function (6 motors + 2 PTO motors when engaged)
        customDrive(lateral_power, linear_power);

        // Intake controls - automatically use PTO motors when disengaged
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
        else {
            setIdle();
        }
        
        // Scraper toggle (unchanged)
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN)){
            scraper_toggle = !scraper_toggle;
        }
        scraper.set_value(scraper_toggle);
        
        pros::delay(25);
    }
}