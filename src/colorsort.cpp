#include "colorsort.hpp"
#include "config.hpp"
#include <iostream>

// Global color sorting variables
IntakeMode currentIntakeMode = IDLE;
ColorMode currentColorMode = DISABLED;

// Toggles
bool basket_toggle = false;
bool scraper_toggle = false;
bool lower_toggle = false;

// Helper functions to change states (updated with mode tracking)
void setIntakeTop() {
    basket.set_value(false);
    bottomrollers.move_velocity(500);
    midrollers.move_velocity(500);
    backroller.move_velocity(-500);
    currentIntakeMode = INTAKE_TOP;
}

void setMidScoring() {
    basket.set_value(true);
    backroller.move_velocity(500);
    bottomrollers.move_velocity(500);
    midrollers.move_velocity(-500);
    currentIntakeMode = MID_SCORING;
}

void setHighScoring() {
    basket.set_value(true);
    blocker.set_value(true);
    backroller.move_velocity(500);
    bottomrollers.move_velocity(500);
    midrollers.move_velocity(500);
    currentIntakeMode = HIGH_SCORING;

}

void setLowScoring() {
    basket.set_value(true);
    bottomrollers.move_velocity(-500);
    midrollers.move_velocity(-500);
    backroller.move_velocity(500);
    currentIntakeMode = LOW_SCORING;
}

void setIdle() {
    basket.set_value(false);
    blocker.set_value(false);
    bottomrollers.move_velocity(0);
    midrollers.move_velocity(0);
    backroller.move_velocity(0);
    currentIntakeMode = IDLE;
}

/**
 * Color sorting control functions - Single toggle through RED -> BLUE -> DISABLED
 */
void toggleColorMode() {
    switch (currentColorMode) {
        case DISABLED:
            currentColorMode = RED_MODE;
            printf("Color sorting: RED MODE\n");
            controller.print(0, 0, "RED MODE");
            break;
        case RED_MODE:
            currentColorMode = BLUE_MODE;
            printf("Color sorting: BLUE MODE\n");
            controller.print(0, 0, "BLUE MODE");
            break;
        case BLUE_MODE:
            currentColorMode = DISABLED;
            printf("Color sorting: DISABLED\n");
            controller.print(0, 0, "SORT OFF");
            break;
    }
}

/**
 * Get reading from an optical sensor
 */
OpticalReading getOpticalReading(pros::Optical& sensor) {
    OpticalReading reading;
    reading.proximity = sensor.get_proximity();
    reading.detected = (reading.proximity > PROXIMITY_THRESHOLD);
    
    if (reading.detected) {
        reading.hue = sensor.get_hue();
    } else {
        reading.hue = 0;
    }
    
    return reading;
}

/**
 * Check all three sensors and determine if we should act
 * Returns true if any sensor detects a ball
 */
bool checkAllSensors(OpticalReading& reading1, OpticalReading& reading2, OpticalReading& reading3) {
    return (reading1.detected || reading2.detected || reading3.detected);
}

/**
 * Get the sensor closest to the ball (highest proximity)
 * Returns pointer to the closest reading and sets sensorNumber
 */
OpticalReading* getClosestSensor(OpticalReading& reading1, OpticalReading& reading2, OpticalReading& reading3, int& sensorNumber) {
    OpticalReading* closestSensor = nullptr;
    int highestProximity = 0;
    
    if (reading1.detected && reading1.proximity > highestProximity) {
        closestSensor = &reading1;
        highestProximity = reading1.proximity;
        sensorNumber = 1;
    }
    
    if (reading2.detected && reading2.proximity > highestProximity) {
        closestSensor = &reading2;
        highestProximity = reading2.proximity;
        sensorNumber = 2;
    }
    
    if (reading3.detected && reading3.proximity > highestProximity) {
        closestSensor = &reading3;
        highestProximity = reading3.proximity;
        sensorNumber = 3;
    }
    
    return closestSensor;
}

/**
 * Check if detected color matches target color
 * Returns TRUE only if the detected color matches the target alliance color
 * Returns FALSE if it's the wrong color (should be rejected)
 */
bool isCorrectColor(int detectedHue) {
    if (currentColorMode == DISABLED) {
        printf("Color sorting disabled - accepting all\n");
        return true;
    }
    
    // Calculate distances to both red and blue with improved red wrap-around handling
    int redDistance1 = abs(detectedHue - RED_HUE);
    int redDistance2 = abs(detectedHue - (RED_HUE + 360));
    int redDistance3 = abs((detectedHue + 360) - RED_HUE);
    int redMinDistance = std::min({redDistance1, redDistance2, redDistance3});
    
    // Also check red hues near 360/0 boundary
    int redDistance4 = abs(detectedHue - 360);  // Pure red at 360
    int redDistance5 = abs(detectedHue - 0);    // Pure red at 0
    redMinDistance = std::min({redMinDistance, redDistance4, redDistance5});
    
    int blueDistance = abs(detectedHue - BLUE_HUE);
    
    bool isRedColor = (redMinDistance <= HUE_TOLERANCE);
    bool isBlueColor = (blueDistance <= HUE_TOLERANCE);
    
    printf("Color analysis: hue=%d | Red distance=%d (isRed=%s) | Blue distance=%d (isBlue=%s)\n", 
           detectedHue, redMinDistance, isRedColor ? "YES" : "NO", 
           blueDistance, isBlueColor ? "YES" : "NO");
    
    if (currentColorMode == RED_MODE) {
        // We want red balls, reject blue balls
        if (isRedColor) {
            printf("RED MODE: Detected RED ball - ACCEPT\n");
            return true;
        } else if (isBlueColor) {
            printf("RED MODE: Detected BLUE ball - REJECT\n");
            return false;
        } else {
            printf("RED MODE: Unknown color (hue=%d) - Check your RED_HUE and TOLERANCE values - ACCEPTING for safety\n", detectedHue);
            return true; // Accept unknown colors to avoid losing good balls
        }
    }
    
    if (currentColorMode == BLUE_MODE) {
        // We want blue balls, reject red balls
        if (isBlueColor) {
            printf("BLUE MODE: Detected BLUE ball - ACCEPT\n");
            return true;
        } else if (isRedColor) {
            printf("BLUE MODE: Detected RED ball - REJECT\n");
            return false;
        } else {
            printf("BLUE MODE: Unknown color (hue=%d) - Check your BLUE_HUE and TOLERANCE values - ACCEPTING for safety\n", detectedHue);
            return true; // Accept unknown colors to avoid losing good balls
        }
    }
    
    printf("Unknown color mode: %d - defaulting to ACCEPT\n", currentColorMode);
    return true;
}

/**
 * Accept ball - continue with current intake mode
 */
void acceptBall() {
    printf("ACCEPT: Correct color - continuing intake\n");
    controller.print(2, 0, "ACCEPT");
}

/**
 * Reject ball - back rollers ALWAYS reverse to eject
 */
void rejectBall() {
    
    
    // Stop intake motion and reverse back roller
    bottomrollers.move_velocity(600);      // Stop bottom intake
    midrollers.move_velocity(600);         // Stop mid intake
    backroller.move_velocity(600);      // ALWAYS reverse to eject
    
    // Wait for ejection
    pros::delay(515);
    
    // Return to intake top mode
    setIntakeTop();
    
    printf("Ball ejected, returned to intake mode\n");
}

/**
 * Main color sorting update function
 * Uses the sensor closest to the ball (highest proximity) to make the decision
 * Now supports 3 sensors for maximum coverage
 */
void updateColorSorting() {
    // Only sort if in INTAKE_TOP mode and color mode is not DISABLED
    if (currentIntakeMode != INTAKE_TOP || currentColorMode == DISABLED) {
        return;
    }
    
    // Get readings from all three sensors
    OpticalReading reading1 = getOpticalReading(optical1);
    OpticalReading reading2 = getOpticalReading(optical2);
    OpticalReading reading3 = getOpticalReading(optical3);
    
    // DEBUG: Print sensor readings even when no ball is detected
    static int debugCounter = 0;
    debugCounter++;
    if (debugCounter % 100 == 0) { // Print every 100 cycles to avoid spam
        printf("DEBUG - S1: prox=%d, hue=%d | S2: prox=%d, hue=%d | S3: prox=%d, hue=%d | Threshold=%d | ColorMode=%d\n", 
               reading1.proximity, reading1.hue, reading2.proximity, reading2.hue, 
               reading3.proximity, reading3.hue, PROXIMITY_THRESHOLD, currentColorMode);
    }
    
    // Check if any sensor detects a ball
    if (checkAllSensors(reading1, reading2, reading3)) {
        // Find which sensor is closest to the ball (highest proximity = closer)
        int sensorNumber = 0;
        OpticalReading* closestSensor = getClosestSensor(reading1, reading2, reading3, sensorNumber);
        
        if (closestSensor == nullptr) {
            printf("Error: No closest sensor found despite detection\n");
            return;
        }
        
        // Show detection info
        printf("Ball detected by sensor %d (prox: %d) | Other sensors: S1=%d, S2=%d, S3=%d\n", 
               sensorNumber, closestSensor->proximity,
               reading1.proximity, reading2.proximity, reading3.proximity);
        
        printf("Decision made by sensor %d: Hue=%d, Proximity=%d\n", 
               sensorNumber, closestSensor->hue, closestSensor->proximity);
        
        // DEBUG: Show color mode and expected hues
        printf("Current color mode: %d (0=RED, 1=BLUE, 2=DISABLED)\n", currentColorMode);
        if (currentColorMode == RED_MODE) {
            printf("Looking for RED hue around %d (tolerance ±%d)\n", (int)RED_HUE, (int)HUE_TOLERANCE);
        } else if (currentColorMode == BLUE_MODE) {
            printf("Looking for BLUE hue around %d (tolerance ±%d)\n", (int)BLUE_HUE, (int)HUE_TOLERANCE);
        }
        
        // Brief pause for stable reading
        pros::delay(50);
        
        // Get a second reading for stability from the same sensor
        OpticalReading secondReading;
        if (sensorNumber == 1) secondReading = getOpticalReading(optical1);
        else if (sensorNumber == 2) secondReading = getOpticalReading(optical2);
        else secondReading = getOpticalReading(optical3);
        
        printf("Second reading from sensor %d: Hue=%d, Proximity=%d\n", 
               sensorNumber, secondReading.hue, secondReading.proximity);
        
        // Use the second reading for decision if it's still detecting
        int finalHue = secondReading.detected ? secondReading.hue : closestSensor->hue;
        
        // Make sorting decision based on the closest sensor's reading
        bool shouldAccept = isCorrectColor(finalHue);
        printf("Color check: Hue=%d, shouldAccept=%s\n", finalHue, shouldAccept ? "YES" : "NO");
        
        if (shouldAccept) {
            acceptBall();
            printf("Sensor %d: ACCEPT decision (hue=%d)\n", sensorNumber, finalHue);
        } else {
            rejectBall();
            printf("Sensor %d: REJECT decision (hue=%d)\n", sensorNumber, finalHue);
        }
        
        // Wait to avoid multiple detections
        pros::delay(300);
    }
}

/**
 * Initialize color sorting system
 */
void initializeColorSorting() {
    setIdle();
    currentColorMode = DISABLED;
    printf("Integrated intake + color sorting system ready\n");
}