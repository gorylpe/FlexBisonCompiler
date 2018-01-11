#pragma once

#include <cln/cln.h>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include "Variable.h"
#include "Position.h"
#include "Utils.h"

using namespace std;
using namespace cln;

class MemoryManager {
public:
    cl_I memoryPtr = 0;
    map<string, Variable*> variables;
    vector<Variable*> stack;

    void addVariable(Position *pos, const string &pid){
        if (this->variables.count(pid) == 1){
            poserror(pos, "variable redeclaration");
            return;
        }
        auto vdec = new Variable(pid, this->memoryPtr, true);
        this->variables[pid] = vdec;
        this->memoryPtr++;
    }

    void addArrayVariable(Position *posPid, Position *posSize, const string &pid, const cl_I &size){
        if (this->variables.count(pid) == 1){
            poserror(posPid, "variable redeclaration");
            return;
        }

        if (size <= 0){
            poserror(posSize, "array size <= 0");
            return;
        }
        auto vdec = new Variable(pid, size, this->memoryPtr, true);
        this->variables[pid] = vdec;
        this->memoryPtr += size;
    }

    Variable* getVariable(const string &pid){
        if (this->variables.count(pid) == 0){
            if(this->stack.empty())
                return nullptr;
            else{
                for(auto var : this->stack){
                    if(var->pid == pid){
                        return var;
                    }
                }
                return nullptr;
            }
        }
        return this->variables[pid];
    }

    Variable* pushTempNamedVariable(const string& name, bool modifiable){
        if (this->variables.count(name) > 0){
            return nullptr;
        }
        auto var = new Variable(name, this->memoryPtr, modifiable);
        this->memoryPtr++;
        this->stack.push_back(var);

        return var;
    }

    Variable* pushTempVariable(){
        stringstream ss;
        ss << "#temp" << stack.size(); //using forbidden sign to prevent duplication name
        auto var = new Variable(ss.str(), this->memoryPtr, false);
        this->memoryPtr++;
        this->stack.push_back(var);

        return var;
    }

    void popTempVariable(){
        auto var = this->stack.back();
        delete var;
        this->memoryPtr--;
        this->stack.pop_back();
    }
};
