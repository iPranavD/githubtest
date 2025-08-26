#include "colorsort.hpp"
#include "config.hpp"
#include <iostream>

// Global color sorting variables


// Toggles
bool park_toggle = false;
bool scraper_toggle = false;
bool mid_toggle = false;

// Helper functions to change states (updated with mode tracking)
void setIntakeTop() {
    bottomrollers.move_velocity(600);
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
}

