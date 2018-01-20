#pragma once

#include <cln/cln.h>
#include <vector>
#include <sstream>
#include <vector>
#include <map>
#include "Position.h"

using namespace std;
using namespace cln;

class Variable {
public:
    enum Type {
        PID = 0,
        PIDNUM
    };

    Type type;

    string pid;
    cl_I size;
    cl_I memoryPtr;

    bool modifiable;
    bool initialized;

    const cl_I nestetLoopMultiplier = 10;
    cl_I useNumber;

    explicit Variable(string pid, const cl_I& memoryPtr, bool modifiable)
    :type(Type::PID)
    ,pid(pid)
    ,memoryPtr(memoryPtr)
    ,modifiable(modifiable)
    ,initialized(false)
    ,useNumber(0){
        cerr << "Declaring variable " << " name " << pid << endl;
    }

    explicit Variable(string pid, const cl_I& size, const cl_I& memoryPtr, bool modifiable)
    :type(Type::PIDNUM)
    ,size(size)
    ,pid(pid)
    ,memoryPtr(memoryPtr)
    ,modifiable(modifiable)
    ,initialized(false)
    ,useNumber(0) {}

    ~Variable(){
        cerr << "Deleting variable " << pid << endl;
    }

    bool isModifiable(){
        return this->modifiable;
    }

    void initialize(){
        if(this->type == PID)
            this->initialized = true;
    }

    bool isInitialized(){
        if(this->type == PID)
            return this->initialized;
        return false;
    }

    void addUsage(const cl_I& numberOfNestedLoops) {
        useNumber += expt(nestetLoopMultiplier, numberOfNestedLoops);
    }

    cl_I getUsage() const {
        return useNumber;
    }
};