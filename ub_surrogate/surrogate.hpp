#pragma once

#include "../lib/instance.hpp"
#include "../lib/solution.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

struct SurrogateOut
{
    SurrogateOut(Info* info): info(info) {}
    Info* info;
    bool verbose;
    Profit ub = 0;
    ItemIdx bound = 0;
    Weight multiplier = 0;
};

SurrogateOut ub_surrogate(const Instance& instance, Profit lb, Info* info = NULL);

