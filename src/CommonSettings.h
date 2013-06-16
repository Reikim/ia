#ifndef COMMON_SETTINGS_H
#define COMMON_SETTINGS_H

const unsigned int PLAYER_NAME_MAX_LENGTH = 12;

const int MAP_X_CELLS       = 80;
const int MAP_Y_CELLS       = 26;
const int MAP_X_CELLS_HALF  = MAP_X_CELLS / 2;
const int MAP_Y_CELLS_HALF  = MAP_Y_CELLS / 2;

const int PLAYER_CLVL_MAX = 30;

const int DELAY_PLAYER_WAITING        = 2;
const int DELAY_PLAYER_UNABLE_TO_ACT  = 7;

const int CAST_FROM_MEMORY_CHANCE_LIM   = 95;
const int CAST_FROM_MEMORY_MTH_BON_DIV  = 3;

const int FIRST_CAVERN_LEVEL  = 20;
const int LAST_CAVERN_LEVEL   = 29;
const int LAST_ROOM_AND_CORRIDOR_LEVEL = FIRST_CAVERN_LEVEL - 2;

const int MIN_DLVL_NASTY_TRAPS = 7;

const int FOV_STANDARD_RADI_INT     = 6;
const int FOV_STANDARD_WIDTH_INT    = 2 * FOV_STANDARD_RADI_INT + 1;
const double FOV_STANDARD_RADI_DB   = double(FOV_STANDARD_RADI_INT);
const double FOV_STANDARD_WIDTH_DB  = double(FOV_STANDARD_WIDTH_INT);
const int FOV_MAX_RADI_INT          = FOV_STANDARD_RADI_INT * 2;
const int FOV_MAX_WIDTH_INT         = 2 * FOV_MAX_RADI_INT + 1;
const double FOV_MAX_RADI_DB        = double(FOV_MAX_RADI_INT);
const double FOV_MAX_WIDTH_DB       = double(FOV_MAX_WIDTH_INT);

const int THROWING_RANGE_LIMIT      = FOV_STANDARD_RADI_INT + 3;

const int DYNAMITE_FUSE_TURNS = 5;

const int SHOCK_TAKEN_FROM_CASTING_SPELLS = 16;

const int NUMBER_OF_MACHINEGUN_PROJECTILES_PER_BURST          = 5;
const int NUMBER_OF_CELLJUMPS_BETWEEN_MACHINEGUN_PROJECTILES  = 2;

const int CHANCE_TO_DESTROY_COMMON_ITEMS_ON_DROP = 50;

#endif