#pragma once


#include <vector>
#include <unordered_map>
#include "Identifier.h"

using namespace std;

class Assignment;

class IdentifierAssignments{
    bool isInMap(int ssaNum){
        return assignments.find(ssaNum) != assignments.end();
    }

public:
    unordered_map<int, Assignment*> assignments;

    explicit IdentifierAssignments() = default;

    bool hasAssignment(int ssaNum){
        return isInMap(ssaNum);
    }

    Assignment* getAssignment(int ssaNum){
        if(isInMap(ssaNum))
            return assignments[ssaNum];
        return nullptr;
    }

    void addAssignment(Assignment* assignment, int ssaNum){
        assignments[ssaNum] = assignment;
    }
};


class IdentifiersAssignmentsHelper {
    void initIdentifierAssignments(const string &pid){
        if(assignments.find(pid) == assignments.end()){
            assignments[pid] = new IdentifierAssignments();
        }
    }
public:
    IdentifiersAssignmentsHelper() = default;

    map<string, IdentifierAssignments*> assignments;

    bool hasAssignment(string pid, int ssaNum){
        initIdentifierAssignments(pid);
        return assignments.at(pid)->hasAssignment(ssaNum);
    }

    void addAssignment(Assignment* assignment, string pid, int ssaNum){
        initIdentifierAssignments(pid);
        assignments.at(pid)->addAssignment(assignment, ssaNum);
    }

    Assignment* getAssignment(string pid, int ssaNum){
        initIdentifierAssignments(pid);
        return assignments.at(pid)->getAssignment(ssaNum);
    }
};