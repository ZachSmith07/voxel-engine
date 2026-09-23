#include "PlayerStats.h"

PlayerStats::PlayerStats()
    : name("Zach"), health(14), maxHealth(20), level(50), isMale(true) {}

float PlayerStats::CalculateMovementSpeed()
{
    float constantSpeed = 12.0f;
    float genderMultiple = isMale ? 1.0f : 0.9f;
    float levelMultiple = 0.96f + 0.04f * level;
    float footwearMultiple = 1.0f;

    return genderMultiple * levelMultiple * footwearMultiple * constantSpeed;
}