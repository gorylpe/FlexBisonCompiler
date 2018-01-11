#pragma once

#include "MemoryManager.h"

class MachineContext {
    MachineContext()
            :counter(0){ }
public:
    static MachineContext& getInstance()
    {
        static MachineContext instance;
        return instance;
    }


    MachineContext(MachineContext const&) = delete;
    void operator=(MachineContext const&) = delete;

    cl_I counter;

    void increaseCounter(){
        this->counter++;
    }

    void increaseCounter(cl_I& value){
        this->counter += value;
    }

    cl_I getLineCounter(){
        return this->counter;
    }

    cl_I jump(cl_I& length){
        this->counter += length;
    }

    cl_I jumpDirect(cl_I& newCounter){
        this->counter = newCounter;
    }

    MemoryManager mem;
};