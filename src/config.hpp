#pragma once
#include "main.h"
#include "lemlib/api.hpp"

// External declarations for hardware components

// Controller
extern pros::Controller controller;

// Motor groups
extern pros::MotorGroup rightMotors;
extern pros::MotorGroup leftMotors;

// Individual motors
extern pros::Motor backroller;
extern pros::Motor midrollers;
extern pros::Motor bottomrollers;

// Sensors
extern pros::Optical optical1;
extern pros::Optical optical2;
extern pros::Optical optical3;
extern pros::Imu imu;

// Pneumatics
extern pros::adi::DigitalOut basket;
extern pros::adi::DigitalOut scraper;
extern pros::adi::DigitalOut lower_basket;
extern pros::adi::DigitalOut instapark;
extern pros::adi::DigitalOut blocker;

// LemLib chassis
extern lemlib::Chassis chassis;

// Configuration functions
void initializeHardware();
void setheading(float degrees, int maxspeed, int timeout = 0);