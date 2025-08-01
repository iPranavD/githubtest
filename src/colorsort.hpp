#pragma once
#include "main.h"

// Color sorting constants - Updated for better detection
constexpr float RED_HUE = 10;        // Red balls are often around 0-20
constexpr float BLUE_HUE = 220;      // Blue balls are often around 200-240
constexpr float HUE_TOLERANCE = 30;  // Increased tolerance for better detection
const int PROXIMITY_THRESHOLD = 200; // Object detection threshold

// Color sorting enums
enum IntakeMode {
    INTAKE_TOP,
    MID_SCORING,
    HIGH_SCORING,
    LOW_SCORING,
    IDLE
};

enum ColorMode {
    RED_MODE,
    BLUE_MODE,
    DISABLED
};

// External declarations for global variables
extern IntakeMode currentIntakeMode;
extern ColorMode currentColorMode;
extern bool basket_toggle;
extern bool scraper_toggle;
extern bool lower_toggle;

// Timing settings
const int REJECT_TIME = 600; // Time to reject wrong color (ms)

// Intake mode functions
void setIntakeTop();
void setMidScoring();
void setHighScoring();
void setLowScoring();
void setIdle();

// Color sorting functions
void toggleColorMode();
bool isCorrectColor(int detectedHue);
void acceptBall();
void rejectBall();
void updateColorSorting();
void initializeColorSorting();

// Dual optical sensor functions
struct OpticalReading {
    int hue;
    int proximity;
    bool detected;
};

OpticalReading getOpticalReading(pros::Optical& sensor);
bool checkAllSensors(OpticalReading& reading1, OpticalReading& reading2, OpticalReading& reading3);
OpticalReading* getClosestSensor(OpticalReading& reading1, OpticalReading& reading2, OpticalReading& reading3, int& sensorNumber);