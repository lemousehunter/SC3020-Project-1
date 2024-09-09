// File: Record.h
#pragma once
#include <string>
#include <ctime>

struct Record {
    std::string game_date_est;
    std::string team_id_home;
    int pts_home;
    float fg_pct_home;
    float ft_pct_home;
    float fg3_pct_home;
    int ast_home;
    int reb_home;
    bool home_team_wins;
};
