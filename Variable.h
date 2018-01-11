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
    map<cl_I, bool> numinitialized;

    explicit Variable(const string& pid, const cl_I& memoryPtr, bool modifiable)
    :type(Type::PID)
    ,pid(pid)
    ,memoryPtr(memoryPtr)
    ,modifiable(modifiable)
    ,initialized(false){
        cerr << "Declaring variable " << " name " << pid << endl;

    }

    explicit Variable(const string& pid, const cl_I& size, const cl_I& memoryPtr, bool modifiable)
    :type(Type::PIDNUM)
    ,pid(pid)
    ,size(size)
    ,memoryPtr(memoryPtr)
    ,modifiable(modifiable)
    ,initialized(false){
        cerr << "Declaring array name " << pid << " size " << size << endl;
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
};