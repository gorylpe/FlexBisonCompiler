#pragma once

#include <cln/cln.h>
#include <vector>
#include <sstream>
#include <vector>
#include <map>
#include "../Position.h"

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

    const cl_I nestedLoopMultiplier = 10;
    cl_I useNumber;

    explicit Variable(string pid, const cl_I& memoryPtr, bool modifiable)
    :type(Type::PID)
    ,pid(pid)
    ,memoryPtr(memoryPtr)
    ,modifiable(modifiable)
    ,initialized(false)
    ,useNumber(0){
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Declaring variable " << pid << endl;
    #endif
    }

    explicit Variable(string pid, const cl_I& size, const cl_I& memoryPtr, bool modifiable)
    :type(Type::PIDNUM)
    ,size(size)
    ,pid(pid)
    ,memoryPtr(memoryPtr)
    ,modifiable(modifiable)
    ,initialized(false)
    ,useNumber(0){
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Deleting array " << pid << "[" << size << "]" << endl;
    #endif
    }

    ~Variable(){
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Deleting variable " << pid << endl;
    #endif
    }

    bool isModifiable(){
        return this->modifiable;
    }

    void initialize(){
        this->initialized = true;
    }

    bool isInitialized(){
        return this->initialized;
    }

    void addUsage(const cl_I& numberOfNestedLoops) {
        useNumber += expt(nestedLoopMultiplier, numberOfNestedLoops);
    }

    cl_I getUsage() const {
        return useNumber;
    }
};