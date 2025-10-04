#include "main.h"
#include "lemlib/api.hpp"

// Controller
pros::Controller controller(pros::E_CONTROLLER_MASTER);

// Dedicated drivetrain motors (always drivetrain - 6 motors)
pros::MotorGroup leftMotors({1, -2, -4}, pros::MotorGearset::blue);
pros::MotorGroup rightMotors({-10, 9, 7}, pros::MotorGearset::blue);

// PTO motors (switch between intake and drivetrain - 2 motors)
// These are your ONLY intake motors when disengaged
pros::Motor pto_left(3, pros::MotorGearset::blue);   // PTO motor for left side
pros::Motor pto_right(8, pros::MotorGearset::blue);   // PTO motor for right side

// Pneumatics
pros::adi::DigitalOut scraper('B');
pros::adi::DigitalOut pto_piston('A');
pros::adi::DigitalOut park_axle('C');
pros::adi::DigitalOut instapark('D');
pros::adi::DigitalOut snacky('E');

std::atomic_bool pto_switch{false};

// Sensors and tracking wheels
pros::Imu imu(5);

lemlib::Drivetrain drivetrain(&leftMotors,
                              &rightMotors,
                              6,
                              lemlib::Omniwheel::NEW_4,
                              400,
                              2
);

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
    // Set brake modes
    pto_left.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    pto_right.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    
    printf("Hardware initialization complete\n");
}

void pto() {
    while(true) {
        if(pto_switch) {
            pto_left.move_velocity(0);
            pto_right.move_velocity(0);
            pto_left.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
            pto_right.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
            pros::delay(50);
            pto_piston.set_value(true);
        }
        else {
            pto_left.move_velocity(0);
            pto_right.move_velocity(0);
            pto_left.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
            pto_right.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
            pros::delay(50);
            pto_piston.set_value(false);
        }
        pros::delay(25);
    }
}

void customDrive(int lateral, int linear) {
    // Always use the 6 dedicated drivetrain motors via LemLib
    chassis.arcade(lateral, linear);
    
    if (pto_switch) {
        // 8-motor mode: add PTO motors to drivetrain
        int leftPower = lateral + linear;
        int rightPower = lateral - linear;
        
        pto_left.move(leftPower);
        pto_right.move(rightPower);
    }
}

void initialize() {
    initializeHardware();
    printf("Robot initialization complete\n");

    pros::Task screenTask([&]() {
        while (true) {
            pros::lcd::print(0, "X: %f", chassis.getPose().x);
            pros::lcd::print(1, "Y: %f", chassis.getPose().y);
            pros::lcd::print(2, "Theta: %f", chassis.getPose().theta);
            lemlib::telemetrySink()->info("Chassis pose: {}", chassis.getPose());
            pros::delay(50);
        }
    });
}

void setIntakeTop() {
    if(!pto_switch) {
        pto_right.move_velocity(600);
        pto_left.move_velocity(0);
    }
}

void setHighScoring() {
    if(!pto_switch) {
        pto_right.move_velocity(600);
        pto_left.move_velocity(-600);
    }
}

void setMidScoring() {
    if(!pto_switch) {
        pto_right.move_velocity(600);
        pto_left.move_velocity(0);
    }
}

void setLowScoring() {
    if(!pto_switch) {
        pto_right.move_velocity(-600);
        pto_left.move_velocity(600);
    }
    
}

void setIdle() {
    if(!pto_switch){
        pto_right.move_velocity(0);
        pto_left.move_velocity(0);
    }
}

void disabled() {}
void competition_initialize() {}

// Autonomous functions - work with either 6 or 8 motor drive
void right5() {
    // Add your autonomous code here
}

void left5() {
    // Add your autonomous code here
}

void autonomous() {
    right5();
}

void opcontrol() {
    printf("Driver control started\n");
    pros::Task pto_task(pto);
    
    while (true) {
        static bool scraper_toggle {false};
        static bool park_toggle {false};
        static bool snacky_toggle {false};
        static bool axle_toggle {false};

        // Get controller inputs INSIDE the loop
        int lateral_power = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y); 
        int linear_power = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_X);
        
        // Use custom drive function (6 motors + 2 PTO motors when engaged)
        customDrive(lateral_power, linear_power);

        // Intake controls - automatically use PTO motors when disengaged
        if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L1)) {
            pto_switch = false;
            setIntakeTop();
        }
        else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R2)) {
            pto_switch = false;
            setMidScoring();
        }
        else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_R1)) {
            pto_switch = false;
            setHighScoring();
        }
        else if (controller.get_digital(pros::E_CONTROLLER_DIGITAL_L2)) {
            pto_switch = false;
            setLowScoring();
        }
        else {
            setIdle();
        }
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_UP)) {
            snacky_toggle = !snacky_toggle;
        }
        if(controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_X)) {
            pto_switch = !pto_switch;
        }
        pto_piston.set_value(pto_switch);
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_A)) {
            park_toggle = !park_toggle;
        }
        instapark.set_value(park_toggle);
        if (controller.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN)) {
            scraper_toggle = !scraper_toggle;
        }
        scraper.set_value(scraper_toggle);
        
        pros::delay(25);
    }
}