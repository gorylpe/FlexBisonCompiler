#pragma once

#include "Value.h"

class NumberValueStats {
public:
    vector<Value*> valsLoaded;
    vector<Value*> valsSubtracted;
    vector<Value*> valsAdded;
    int numberOfSubtractions;
    int numberOfLoads;
    int numberOfAdditions;

    NumberValueStats()
    :numberOfAdditions(0)
    ,numberOfSubtractions(0)
    ,numberOfLoads(0){}

    void addSubtraction(Value* val, int num){
        numberOfSubtractions += num;
        valsSubtracted.push_back(val);
    }

    void addLoad(Value* val, int num){
        numberOfLoads += num;
        valsLoaded.push_back(val);
    }

    void addAddition(Value* val, int num){
        numberOfAdditions += num;
        valsAdded.push_back(val);
    }

    string toString() const {
        stringstream ss;
        ss << "Num of subtractions: " << numberOfSubtractions << endl;
        ss << "Num of additions: " << numberOfAdditions << endl;
        ss << "Num of loads: " << numberOfLoads << endl;
        return ss.str();
    }
};
