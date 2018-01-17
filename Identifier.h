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
    Variable* preparedAddressForPidpid;
    bool isPreparedForPidpid;
    cl_I num;

    explicit Identifier(Position* pos, const string& pid)
    :pos(pos)
    ,pid(pid)
    ,type(PID)
    ,isPreparedForPidpid(false){
        cerr << "Creating identifier PID " << pid << endl;
    }

    explicit Identifier(Position* pos, const string& pid, const string& pidpid)
    :pos(pos)
    ,pid(pid)
    ,pidpid(pidpid)
    ,type(PIDPID)
    ,isPreparedForPidpid(false){
        cerr << "Creating identifier PIDPID " << pid << " [" << pidpid << "]" << endl;
    }

    explicit Identifier(Position* pos, const string& pid, const cl_I& num)
    :pos(pos)
    ,pid(pid)
    ,type(PIDNUM)
    ,num(num)
    ,isPreparedForPidpid(false){
        cerr << "Creating identifier PIDNUM " << pid << " [" << num << "]" << endl;
    }

    bool equals(Identifier* ident2){
        if(this->type != ident2->type){
            return false;
        } else {
            switch(this->type){
                case PID:
                    return pid == ident2->pid;
                case PIDNUM:
                    return pid == ident2->pid && num == ident2->num;
                case PIDPID:
                    return pid == ident2->pid && pidpid == ident2->pidpid;
            }
        }
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

    void prepareIfNeeded(){
        if(this->type == Identifier::Type::PIDPID && !isPreparedForPidpid){
            Variable* pidV = this->getPidVariable();
            Variable* pidpidV = this->getPidPidVariable();

            Number pidAddr(pos, pidV->memoryPtr);
            pidAddr.loadToAccumulator();

            machine.ADD(pidpidV->memoryPtr);

            Variable* ptr = memory.pushTempVariable();
            machine.STORE(ptr->memoryPtr);

            preparedAddressForPidpid = ptr;
            isPreparedForPidpid = true;

            cerr << "Identifier " << pid << "[" << pidpid << "] prepared" << endl;
        }
    }

    void unprepareIfNeeded(){
        if(isPreparedForPidpid){
            isPreparedForPidpid = false;
            memory.popTempVariable(); //preparedAddressForPidpid
            preparedAddressForPidpid = nullptr;
            cerr << "Identifier " << pid << "[" << pidpid << "] unprepared" << endl;
        }
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
                if(isPreparedForPidpid){
                    machine.STOREI(preparedAddressForPidpid->memoryPtr);
                } else {
                    //cant ever happen, means mistake in C++ code
                    cerr << "Identifier " << pid << "[" << pidpid << "] STORE error not prepared" << endl;
                    throw exception();
                }
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
                if(isPreparedForPidpid){
                    machine.LOADI(this->preparedAddressForPidpid->memoryPtr);
                } else {
                    //cant ever happen, means mistake in C++ code
                    cerr << "Identifier " << pid << "[" << pidpid << "] LOAD error not prepared" << endl;
                    throw exception();
                }
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
                if(isPreparedForPidpid){
                    machine.ADDI(this->preparedAddressForPidpid->memoryPtr);
                } else {
                    //cant ever happen, means mistake in C++ code
                    cerr << "Identifier " << pid << "[" << pidpid << "] ADD error not prepared" << endl;
                    throw exception();
                }
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
                if(isPreparedForPidpid){
                    machine.SUBI(this->preparedAddressForPidpid->memoryPtr);
                } else {
                    //cant ever happen, means mistake in C++ code
                    cerr << "Identifier " << pid << "[" << pidpid << "] SUB error not prepared" << endl;
                    throw exception();
                }
                break;
        }
    }
};