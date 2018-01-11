#pragma once

#include "Utils.h"
#include "MachineContext.h"

inline void GET(stringstream& ss){
    ss << "GET" << endl;
    machine.increaseCounter();
}

inline void PUT(stringstream& ss){
    ss << "PUT"<< endl;
    machine.increaseCounter();
}

inline void LOAD(stringstream& ss, cl_I& memoryPtr){
    ss << "LOAD " << memoryPtr << endl;
    machine.increaseCounter();
}

inline void LOADI(stringstream& ss, cl_I& memoryPtr){
    ss << "LOADI " << memoryPtr << endl;
    machine.increaseCounter();
}

inline void STORE(stringstream& ss, cl_I& memoryPtr){
    ss << "STORE " << memoryPtr << endl;
    machine.increaseCounter();
}

inline void STOREI(stringstream& ss, cl_I& memoryPtr){
    ss << "STOREI " << memoryPtr << endl;
    machine.increaseCounter();
}

inline void ADD(stringstream& ss, cl_I& memoryPtr){
    ss << "ADD " << memoryPtr << endl;
    machine.increaseCounter();
}

inline void ADDI(stringstream& ss, cl_I& memoryPtr){
    ss << "ADDI " << memoryPtr << endl;
    machine.increaseCounter();
}

inline void SUB(stringstream& ss, cl_I& memoryPtr){
    ss << "SUB " << memoryPtr << endl;
    machine.increaseCounter();
}

inline void SUBI(stringstream& ss, cl_I& memoryPtr){
    ss << "SUBI " << memoryPtr << endl;
    machine.increaseCounter();
}

inline void SHR(stringstream& ss){
    ss << "SHR " << endl;
    machine.increaseCounter();
}

inline void SHL(stringstream& ss){
    ss << "SHL" << endl;
    machine.increaseCounter();
}

inline void INC(stringstream& ss){
    ss << "INC" << endl;
    machine.increaseCounter();
}

inline void DEC(stringstream& ss){
    ss << "DEC" << endl;
    machine.increaseCounter();
}

inline void ZERO(stringstream& ss){
    ss << "ZERO" << endl;
    machine.increaseCounter();
}

inline void JUMP(stringstream& ss, cl_I& lineNumber){
    ss << "JUMP " << lineNumber << endl;
    machine.increaseCounter();
}

inline void JZERO(stringstream& ss, cl_I& lineNumber){
    ss << "JZERO " << lineNumber << endl;
    machine.increaseCounter();
}

inline void JODD(stringstream& ss, cl_I& lineNumber){
    ss << "JODD " << lineNumber << endl;
    machine.increaseCounter();
}

inline void HALT(stringstream& ss){
    ss << "HALT" << endl;
    machine.increaseCounter();
}