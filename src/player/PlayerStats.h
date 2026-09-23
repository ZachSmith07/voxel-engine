#ifndef PLAYER_STATS_H
#define PLAYER_STATS_H

#include <string>

class PlayerStats {
public:
    PlayerStats();
    PlayerStats(const std::string& name, int health, int level, int experience);

    
    std::string name;
    int health;
    int maxHealth;
    int level;
    bool isMale;

    float CalculateMovementSpeed();


private:
};

#endif // PLAYER_STATS_H
