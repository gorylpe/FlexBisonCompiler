#pragma once

#include <cln/cln.h>
#include <vector>
#include <sstream>
#include "Variable.h"
#include "MachineContext.h"
#include "Utils.h"
#include "Assembly.h"
#include "Number.h"

using namespace std;
using namespace cln;

class Identifier {
public:
    enum Type {
        PID = 0,
        PIDPID,
        PIDNUM
    };

    Position* pos;
    Type type;
    string pid;
    string pidpid;
    cl_I num;

    explicit Identifier(Position* pos, const string& pid)
    :pos(pos)
    ,pid(pid)
    ,type(PID){
        cerr << "Creating identifier PID " << pid << endl;
    }

    explicit Identifier(Position* pos, const string& pid, const string& pidpid)
    :pos(pos)
    ,pid(pid)
    ,pidpid(pidpid)
    ,type(PIDPID){
        cerr << "Creating identifier PIDPID " << pid << " [" << pidpid << "]" << endl;
    }

    explicit Identifier(Position* pos, const string& pid, const cl_I& num)
    :pos(pos)
    ,pid(pid)
    ,type(PIDNUM)
    ,num(num){
        cerr << "Creating identifier PIDNUM " << pid << " [" << num << "]" << endl;
    }

    string toString(){
        string type;
        switch(this->type){
            case PID:
                type = "pid";
                break;
            case PIDPID:
                type = "pidpid";
                break;
            case PIDNUM:
                type = "pidnum";
                break;
        }
        return "identifier " + type + " " + this->pid + (this->type == PIDPID ? this->pidpid : "");
    }

    Variable* getPidVariable(){
        Variable* pidV = memory.getVariable(this->pid);

        switch(this->type){
            case PID:{
                if (pidV == nullptr){
                    poserror(this->pos, "variable \"" + this->pid + "\" not declared");
                    return nullptr;
                }

                if (pidV->type == Variable::Type::PIDNUM){
                    poserror(this->pos, "\"" + this->pid + "\" is array, not variable");
                    return nullptr;
                }
            }
            break;
            case PIDPID:{
                if (pidV == nullptr){
                    poserror(this->pos, "variable \"" + this->pid + "\" not declared");
                    return nullptr;
                }

                if (pidV->type == Variable::Type::PID){
                    poserror(this->pos, "\"" + this->pid + "\" is variable, not array");
                    return nullptr;
                }
            }
            break;
            case PIDNUM:{
                if (pidV == nullptr){
                    poserror(this->pos, "variable \"" + this->pid + "\" not declared");
                    return nullptr;
                }
                if (pidV->type == Variable::Type::PID){
                    poserror(this->pos, "\"" + this->pid + "\" is variable, not array");
                    return nullptr;
                }
                if (num < 0 || num >= pidV->size){
                    stringstream ss;
                    ss << "num is out of range ";
                    if (num < 0) {
                        ss << num << " < 0";
                    } else {
                        ss << num << " >= " << pidV->size;
                    }
                    poserror(this->pos, ss.str());
                    return nullptr;
                }
            }
        }

        return pidV;
    }

    Variable* getPidPidVariable(){
        Variable* pidpidV = nullptr;

        if(this->type == PIDPID){
            pidpidV = memory.getVariable(this->pidpid);
            if (pidpidV == nullptr) {
                poserror(this->pos, "variable \"" + this->pidpid + "\" not declared");
                return nullptr;
            }

            if (pidpidV->type == Variable::Type::PIDNUM) {
                poserror(this->pos, "\"" + this->pidpid + "\" is variable, not array");
                return nullptr;
            }
        }

        return pidpidV;
    }

    void storeFromAccumulator(){
        Variable* pidV = this->getPidVariable();
        Variable* pidpidV = this->getPidPidVariable();

        if(!pidV->isModifiable()){
            poserror(this->pos, "assignment to non-modifiable variable");
        }

        switch (this->type) {
            case Identifier::Type::PID:
                pidV->initialize();
                machine.STORE(pidV->memoryPtr);
                break;
            case Identifier::Type::PIDNUM: {
                pidV->initialize();
                cl_I arrayIndexMemoryPtr = pidV->memoryPtr + this->num;
                machine.STORE(arrayIndexMemoryPtr);
                break;
            }
            case Identifier::Type::PIDPID:
                pidV->initialize();
                if(!pidpidV->isInitialized()){
                    poserror(this->pos, "variable is not initialized");
                }
                Variable* acc = memory.pushTempVariable();
                machine.STORE(acc->memoryPtr);

                Number pidAddr(this->pos, pidV->memoryPtr);
                pidAddr.loadToAccumulator();

                machine.ADD(pidpidV->memoryPtr);

                Variable* ptr = memory.pushTempVariable();
                machine.STORE(ptr->memoryPtr);

                machine.LOAD(acc->memoryPtr);

                machine.STOREI(ptr->memoryPtr);
                memory.popTempVariable(); //acc
                memory.popTempVariable(); //ptr
                break;
        }
    }

    void loadToAccumulator(){
        Variable* pidV = this->getPidVariable();
        Variable* pidpidV = this->getPidPidVariable();

        switch (this->type){
            case Identifier::Type::PID:
                if(!pidV->isInitialized()){
                    poserror(this->pos, "variable " + this->pid + " is not initialized");
                }
                machine.LOAD(pidV->memoryPtr);
                break;
            case Identifier::Type::PIDNUM: {
                cl_I arrayIndexMemoryPtr = pidV->memoryPtr + this->num;
                machine.LOAD(arrayIndexMemoryPtr);
                break;
            }
            case Identifier::Type::PIDPID:
                if(!pidpidV->isInitialized()){
                    poserror(this->pos, "variable " + this->pid + " is not initialized");
                }
                Number pidAddr(this->pos, pidV->memoryPtr);
                pidAddr.loadToAccumulator();

                machine.ADD(pidpidV->memoryPtr);

                Variable* ptr = memory.pushTempVariable();
                machine.STORE(ptr->memoryPtr);

                machine.LOADI(ptr->memoryPtr);
                memory.popTempVariable(); //ptr
                break;
        }
    }

    void addToAccumulator(){
        Variable* pidV = this->getPidVariable();
        Variable* pidpidV = this->getPidPidVariable();

        switch (this->type){
            case Identifier::Type::PID:
                if(!pidV->isInitialized()){
                    poserror(this->pos, "variable " + this->pid + " is not initialized");
                }
                machine.ADD(pidV->memoryPtr);
                break;
            case Identifier::Type::PIDNUM: {
                cl_I arrayIndexMemoryPtr = pidV->memoryPtr + this->num;
                machine.ADD(arrayIndexMemoryPtr);
                break;
            }
            case Identifier::Type::PIDPID:
                if(!pidpidV->isInitialized()){
                    poserror(this->pos, "variable " + this->pid + " is not initialized");
                }
                Variable* acc = memory.pushTempVariable();
                machine.STORE(acc->memoryPtr);

                Number pidAddr(this->pos, pidV->memoryPtr);
                pidAddr.loadToAccumulator();

                machine.ADD(pidpidV->memoryPtr);

                Variable* ptr = memory.pushTempVariable();
                machine.STORE(ptr->memoryPtr);

                machine.LOAD(acc->memoryPtr);

                machine.ADDI(ptr->memoryPtr);
                memory.popTempVariable(); //acc
                memory.popTempVariable(); //ptr
                break;
        }
    }

    void subFromAccumulator(){
        Variable* pidV = this->getPidVariable();
        Variable* pidpidV = this->getPidPidVariable();

        switch (this->type){
            case Identifier::Type::PID:
                if(!pidV->isInitialized()){
                    poserror(this->pos, "variable " + this->pid + " is not initialized");
                }
                machine.SUB(pidV->memoryPtr);
                break;
            case Identifier::Type::PIDNUM: {
                cl_I arrayIndexMemoryPtr = pidV->memoryPtr + this->num;
                machine.SUB(arrayIndexMemoryPtr);
                break;
            }
            case Identifier::Type::PIDPID:
                if(!pidpidV->isInitialized()){
                    poserror(this->pos, "variable " + this->pid + " is not initialized");
                }
                Variable* acc = memory.pushTempVariable();
                machine.STORE(acc->memoryPtr);

                Number pidAddr(this->pos, pidV->memoryPtr);
                pidAddr.loadToAccumulator();

                machine.ADD(pidpidV->memoryPtr);

                Variable* ptr = memory.pushTempVariable();
                machine.STORE(ptr->memoryPtr);

                machine.LOAD(acc->memoryPtr);

                machine.SUBI(ptr->memoryPtr);
                memory.popTempVariable(); //acc
                memory.popTempVariable(); //ptr
                break;
        }
    }
};