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

    set<int> ssaNums;
    set<int> ssaNumsPidpid;

    explicit Identifier(Position* pos, string pid)
    :pos(pos)
    ,pid(pid)
    ,type(PID)
    ,isPreparedForPidpid(false){
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Creating identifier PID " << toString() << endl;
    #endif
    }

    explicit Identifier(Position* pos, string pid, string pidpid)
    :pos(pos)
    ,pid(pid)
    ,pidpid(pidpid)
    ,type(PIDPID)
    ,isPreparedForPidpid(false){
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Creating identifier PIDPID " << toString() << endl;
    #endif
    }

    explicit Identifier(Position* pos, string pid, const cl_I& num)
    :pos(pos)
    ,pid(pid)
    ,type(PIDNUM)
    ,num(num)
    ,isPreparedForPidpid(false){
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Creating identifier PIDNUM " << toString() << endl;
    #endif
    }

    Identifier(const Identifier& ident2)
    :pos(new Position(*ident2.pos))
    ,pid(string(ident2.pid))
    ,type(ident2.type)
    ,isPreparedForPidpid(ident2.isPreparedForPidpid)
    ,ssaNums(ident2.ssaNums)
    ,ssaNumsPidpid(ident2.ssaNumsPidpid){
        switch(ident2.type){
            case PIDNUM:
                num = cl_I(ident2.num);
                break;
            case PIDPID:
                pidpid = string(ident2.pidpid);
                break;
        }
    }

    Identifier* clone(){
        return new Identifier(*this);
    }

    //todo DO REFRATOR WITH THIS GETTERS
    bool isTypePID() {
        return type == PID;
    }

    bool isTypePIDNUM() {
        return type == PIDNUM;
    }

    bool isTypePIDPID() {
        return type == PIDPID;
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
        stringstream ss;
        ss << this->pid;
        if(this->type == PIDNUM){
            ss << "[" << this->num << "]";
        } else if(this->type == PIDPID){
            ss << "[" << this->pidpid;

            ss << "\033[1;32m";
            if(isUniquelyDefiniedPidpid()){
                ss << getUniqueSSANumPidpid();
            } else if (ssaNumsPidpid.size() > 1) {
                ss << "(";
                bool first = true;
                for(const auto& ssa : ssaNumsPidpid){
                    if(!first)
                        ss << ", ";
                    first = false;
                    ss << ssa;
                }
                ss << ")";
            }
            ss << "\033[0m";

            ss << "]";
        }

        ss << "\033[1;32m";
        if(isUniquelyDefinied()){
            ss << getUniqueSSANum();
        } else if (ssaNums.size() > 1) {
            ss << "(";
            bool first = true;
            for(const auto& ssa : ssaNums){
                if(!first)
                    ss << ", ";
                first = false;
                ss << ssa;
            }
            ss << ")";
        }
        ss << "\033[0m";
        return ss.str();
    }

    void setSSANums(set<int> newSsaNums) {
        ssaNums = move(newSsaNums);
    }

    set<int>& getSSANums() {
        return ssaNums;
    }

    bool isUniquelyDefinied(){
        return ssaNums.size() == 1;
    }

    int getUniqueSSANum() {
        return *ssaNums.begin();
    }

    void setSSANumsPidpid(set<int> newSsaNums) {
        ssaNumsPidpid = move(newSsaNums);
    }

    set<int>& getSSANumsPidpid() {
        return ssaNumsPidpid;
    }

    bool isUniquelyDefiniedPidpid(){
        return ssaNumsPidpid.size() == 1;
    }

    int getUniqueSSANumPidpid() {
        return *ssaNumsPidpid.begin();
    }

    void checkPids(Variable* pidV, Variable* pidpidV){
        switch(this->type){
            case PID:{
                if (pidV == nullptr){
                    poserror(this->pos, "variable \"" + this->pid + "\" not declared");
                    return;
                }

                if (pidV->type == Variable::Type::PIDNUM){
                    poserror(this->pos, "\"" + this->pid + "\" is array, not variable");
                }
            }
                break;
            case PIDPID:{
                if (pidV == nullptr){
                    poserror(this->pos, "variable \"" + this->pid + "\" not declared");
                    return;
                }

                if (pidV->type == Variable::Type::PID){
                    poserror(this->pos, "\"" + this->pid + "\" is variable, not array");
                }

                if (pidpidV == nullptr) {
                    poserror(this->pos, "variable \"" + this->pidpid + "\" not declared");
                    return;
                }

                if (pidpidV->type == Variable::Type::PIDNUM) {
                    poserror(this->pos, "\"" + this->pidpid + "\" has to be variable, not array");
                }
            }
                break;
            case PIDNUM:{
                if (pidV == nullptr){
                    poserror(this->pos, "variable " + this->pid + " not declared");
                    return;
                }
                if (pidV->type == Variable::Type::PID){
                    poserror(this->pos, this->pid + " is variable, not array");
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
                }
            }
            break;
        }
    }

    void semanticAnalysisGet() {
        Variable *pidV = memory.getVariable(pid);
        Variable *pidpidV = memory.getVariable(pidpid);

        checkPids(pidV, pidpidV);

        if (!pidV->isInitialized()) {
            poserror(pos, "variable " + pid + " is not initialized");
        }

        if (type == PIDPID && !pidpidV->isInitialized()) {
            poserror(pos, "variable " + pidpid + " is not initialized");
        }
    }

    void semanticAnalysisSet(){
        Variable* pidV = memory.getVariable(pid);
        Variable* pidpidV = memory.getVariable(pidpid);

        checkPids(pidV, pidpidV);

        pidV->initialize();

        if (!pidV->isModifiable()) {
            poserror(pos, "assignment to non-modifiable variable");
        }

        if (type == PIDPID && !pidpidV->isInitialized()) {
            poserror(pos, "variable " + pidpid + "is not initialized");
        }
    }

    void replaceValuesWithConst(string pid, cl_I number){
        if(this->type == PIDPID){
            if(pidpid == pid){
                cerr << "Constant in " << toString() << " propagated" << endl;
                this->type = PIDNUM;
                this->num = number;
            }
        }
    }

    Variable* getPidVariable(){
        Variable* pidV = memory.getVariable(pid);

        return pidV;
    }

    Variable* getPidPidVariable(){
        Variable* pidpidV = memory.getVariable(pidpid);

        return pidpidV;
    }

    void prepareIfNeeded(){
        if(this->type == Identifier::Type::PIDPID && !isPreparedForPidpid) {
            Variable *pidV = this->getPidVariable();
            Variable *pidpidV = this->getPidPidVariable();

            //tab start at 0 indirect address is exact one in pidpidV
            if (pidV->memoryPtr == 0) {
                preparedAddressForPidpid = pidpidV;
            }else {
                Number pidAddr(pos, pidV->memoryPtr);
                pidAddr.loadToAccumulator();

                machine.ADD(pidpidV->memoryPtr);

                Variable* ptr = memory.pushTempVariable();
                machine.STORE(ptr->memoryPtr);

                preparedAddressForPidpid = ptr;
            }
            isPreparedForPidpid = true;

            cerr << "Identifier " << pid << "[" << pidpid << "] prepared" << endl;
        }
    }

    void unprepareIfNeeded(){
        if(isPreparedForPidpid){
            isPreparedForPidpid = false;

            Variable *pidV = this->getPidVariable();

            if (pidV->memoryPtr != 0) {
                memory.popTempVariable(); //preparedAddressForPidpid tmp variable
                preparedAddressForPidpid = nullptr;
            }
            cerr << "Identifier " << pid << "[" << pidpid << "] unprepared" << endl;
        }
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops){
        this->getPidVariable()->addUsage(numberOfNestedLoops);
    }

    void storeFromAccumulator(){
        Variable* pidV = this->getPidVariable();
        Variable* pidpidV = this->getPidPidVariable();

        switch (this->type) {
            case Identifier::Type::PID:
                machine.STORE(pidV->memoryPtr);
                break;
            case Identifier::Type::PIDNUM: {
                cl_I arrayIndexMemoryPtr = pidV->memoryPtr + this->num;
                machine.STORE(arrayIndexMemoryPtr);
                break;
            }
            case Identifier::Type::PIDPID:
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
                machine.LOAD(pidV->memoryPtr);
                break;
            case Identifier::Type::PIDNUM: {
                cl_I arrayIndexMemoryPtr = pidV->memoryPtr + this->num;
                machine.LOAD(arrayIndexMemoryPtr);
                break;
            }
            case Identifier::Type::PIDPID:
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
                machine.ADD(pidV->memoryPtr);
                break;
            case Identifier::Type::PIDNUM: {
                cl_I arrayIndexMemoryPtr = pidV->memoryPtr + this->num;
                machine.ADD(arrayIndexMemoryPtr);
                break;
            }
            case Identifier::Type::PIDPID:
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
                machine.SUB(pidV->memoryPtr);
                break;
            case Identifier::Type::PIDNUM: {
                cl_I arrayIndexMemoryPtr = pidV->memoryPtr + this->num;
                machine.SUB(arrayIndexMemoryPtr);
                break;
            }
            case Identifier::Type::PIDPID:
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