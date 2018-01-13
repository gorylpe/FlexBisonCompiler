#pragma once

#include "MemoryManager.h"
#include "Assembly.h"

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

    MemoryManager mem;


    vector<AssemblyLine*> assemblyCode;

    //todo code generation, look for STORE LOAD duplications, JUMP to line after

    cl_I counter;

    void increaseCounter(){
        this->counter++;
    }

    cl_I getLineCounter(){
        return this->counter;
    }

    void setJump(JumpPosition* position){
        position->setPosition(assemblyCode.size());
    }
    
    void GET() {
        assemblyCode.push_back(new GETAssemblyLine());
    }

    void PUT(){
        assemblyCode.push_back(new PUTAssemblyLine());
    }

    void LOAD(cl_I& memoryPtr){
        assemblyCode.push_back(new LOADAssemblyLine(memoryPtr));
    }

    void LOADI(cl_I& memoryPtr){
        assemblyCode.push_back(new LOADIAssemblyLine(memoryPtr));
    }

    void STORE(cl_I& memoryPtr){
        assemblyCode.push_back(new STOREAssemblyLine(memoryPtr));
    }

    void STOREI(cl_I& memoryPtr){
        assemblyCode.push_back(new STOREIAssemblyLine(memoryPtr));
    }

    void ADD(cl_I& memoryPtr){
        assemblyCode.push_back(new ADDAssemblyLine(memoryPtr));
    }

    void ADDI(cl_I& memoryPtr){
        assemblyCode.push_back(new ADDIAssemblyLine(memoryPtr));
    }

    void SUB(cl_I& memoryPtr){
        assemblyCode.push_back(new SUBAssemblyLine(memoryPtr));
    }

    void SUBI(cl_I& memoryPtr){
        assemblyCode.push_back(new SUBIAssemblyLine(memoryPtr));
    }

    void SHR(){
        assemblyCode.push_back(new SHRAssemblyLine());
    }

    void SHL(){
        assemblyCode.push_back(new SHLAssemblyLine());
    }

    void INC(){
        assemblyCode.push_back(new INCAssemblyLine());
    }

    void DEC(){
        assemblyCode.push_back(new DECAssemblyLine());
    }

    void ZERO(){
        assemblyCode.push_back(new ZEROAssemblyLine());
    }

    void JUMP(JumpPosition* position){
        assemblyCode.push_back(new JUMPAssemblyLine(position));
    }

    void JZERO(JumpPosition* position){
        assemblyCode.push_back(new JZEROAssemblyLine(position));
    }

    void JODD(JumpPosition* position){
        assemblyCode.push_back(new JODDAssemblyLine(position));
    }

    void HALT(){
        assemblyCode.push_back(new HALTAssemblyLine());
    }
};