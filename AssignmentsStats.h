#pragma once

#include <string>
#include <map>
#include "cmds/Assignment.h"

using namespace std;

class AssignmentStats{

};

class AssignmentsStats{
public:
    map<string, AssignmentStats> stats;
    map<string, int> identifiersCounters;

    void addAssignment(Assignment* assgs) {};
};