#pragma once

#include <cln/cln.h>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <algorithm>
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
        if(isVariableDeclared(pid)){
            if (this->variables.count(pid) > 0){
                return this->variables[pid];
            } else {
                for(auto var : this->stack){
                    if(var->pid == pid){
                        return var;
                    }
                }
            }
        }
        return nullptr;
    }

    bool isVariableDeclared(const string& pid){
        if(this->variables.count(pid) > 0){
            return true;
        }

        for(auto var : this->stack){
            if(var->pid == pid){
                return true;
            }
        }

        return false;
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

    string memoryToString(){
        stringstream ss;

        for(auto& entry : variables){
            Variable* var = entry.second;

            ss << var->pid << "   ";
            ss << var->memoryPtr;
            if(var->type == Variable::Type::PIDNUM){
                ss << "-" << var->memoryPtr + var->size - 1;
            }
            ss << "   " << var->getUsage();
            ss << endl;
        }

        return ss.str();
    }

    void optimize(){
        optimizeArraysMemoryPointers();
    }

    void optimizeArraysMemoryPointers(){
        cerr << "---ARRAYS MOVING TO MEMORY START OPTIMIZATION---" << endl;
        cerr << memoryToString();

        vector<Variable*> singles;
        for(auto& entry : variables) {
            if(entry.second->type == Variable::Type::PID)
            singles.push_back( entry.second );
        }

        vector<Variable*> arrays;
        for(auto& entry : variables) {
            if(entry.second->type == Variable::Type::PIDNUM)
            arrays.push_back( entry.second );
        }

        //sort based on how many times was used, most used first
        sort(arrays.begin(), arrays.end(), [](const Variable* lhs, const Variable* rhs)
        {
            return lhs->getUsage() > rhs->getUsage();
        });

        if(!arrays.empty()){
            cl_I newMemoryPtr = 0;

            for(auto i = arrays.begin(); i != arrays.end(); ++i){
                Variable* array = *i;
                array->memoryPtr = newMemoryPtr;
                newMemoryPtr += array->size;
            }

            for(auto single : singles){
                single->memoryPtr = newMemoryPtr;
                newMemoryPtr += 1;
            }
        }

        cerr << memoryToString();
        cerr << "---OPTIMIZATION END---" << endl;
    }
};
