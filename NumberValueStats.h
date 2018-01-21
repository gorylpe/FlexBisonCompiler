#pragma once

#include "Value.h"

class NumberValueStats {
public:
    vector<Value*> vals;
    int numberOfSubtractions;
    int numberOfLoads;
    int numberOfAdditions;

    NumberValueStats()
    :numberOfAdditions(0)
    ,numberOfSubtractions(0)
    ,numberOfLoads(0){}

    void addSubtraction(Value* val, int num){
        numberOfSubtractions += num;
        vals.push_back(val);
    }

    void addLoad(Value* val, int num){
        numberOfLoads += num;
        vals.push_back(val);
    }

    void addAddition(Value* val, int num){
        numberOfAdditions += num;
        vals.push_back(val);
    }

    string toString(){
        stringstream ss;
        ss << "Num of occurencies: " << vals.size() << endl;
        ss << "Num of subtractions: " << numberOfSubtractions << endl;
        ss << "Num of additions: " << numberOfAdditions << endl;
        ss << "Num of loads: " << numberOfLoads << endl;
        return ss.str();
    }
};
