#pragma once

#include <set>
#include "MemoryManager.h"
#include "Assembly.h"

class MachineContext {
    MachineContext() = default;
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
    set<JumpPosition*> jumps;

    //todo code generation, look for STORE LOAD duplications, JUMP to line after

    void optimizeContinuousCodeBlocks() {
        unsigned long linesNum = assemblyCode.size();
        bool hasJump[linesNum];
        for (int i = 0; i < linesNum; ++i) {
            hasJump[i] = dynamic_cast<JumpAssemblyLine *>(assemblyCode[i]) != nullptr;
        }

        for (auto jump : jumps) {
            hasJump[jump->getPosition()] = true;
        }

        //debug
        /*for(int i = 0; i < linesNum; ++i){
            cerr << "Line: " << i << " Jump: " << hasJump[i] << endl;
        }*/

        bool toRemove[linesNum];
        for (int i = 0; i < linesNum; ++i) {
            toRemove[i] = false;
        }

        //OPTIMIZE STORE LOADS
        cl_I currentAccumulatorMemoryPtr = -1;
        for (int i = 0; i < linesNum; ++i) {
            if (hasJump[i]) {
                //reset current accumulator memory ptr
                currentAccumulatorMemoryPtr = -1;
            }
            auto storeLine = dynamic_cast<STOREAssemblyLine *>(assemblyCode[i]);
            auto loadLine = dynamic_cast<LOADAssemblyLine *>(assemblyCode[i]);
            if (storeLine != nullptr) {
                currentAccumulatorMemoryPtr = storeLine->getMemoryPtr();
            } else if (loadLine != nullptr) {
                if (loadLine->getMemoryPtr() == currentAccumulatorMemoryPtr &&
                    !hasJump[i]) {
                    toRemove[i] = true;
                }
            }
        }

        //todo optimize INC DEC

        //debug
        /*for (int i = 0; i < linesNum; ++i) {
            cerr << "Line: " << i << " To remove: " << toRemove[i] << endl;
        }*/

        //calculate jump shifts
        int jumpShift[linesNum];
        jumpShift[0] = 0;
        for (int i = 1; i < linesNum; ++i) {
            jumpShift[i] = jumpShift[i - 1] + (toRemove[i] ? 1 : 0);
        }

        //debug
        /*for (int i = 0; i < linesNum; ++i) {
            cerr << "Line: " << i << " Jump shift: " << jumpShift[i] << endl;
        }*/

        //sift jumps
        for (auto jump : jumps) {
            unsigned long position = jump->getPosition();
            //cerr << "Jump old: " << position;
            position -= jumpShift[position];
            //cerr << " new: " << position << endl;
            jump->setPosition(position);
        }

        //remove unused lines
        for(unsigned long i = linesNum - 1; i >= 0; --i){
            if(toRemove[i]){
                assemblyCode.erase(assemblyCode.begin() + i);
            }
        }
    }


    void generateCode(stringstream& ss){
        for(auto line : assemblyCode){
            line->toStringstream(ss);
        }
    }

    void setJumpPosition(JumpPosition *position){
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
        jumps.insert(position);
    }

    void JZERO(JumpPosition* position){
        assemblyCode.push_back(new JZEROAssemblyLine(position));
        jumps.insert(position);
    }

    void JODD(JumpPosition* position){
        assemblyCode.push_back(new JODDAssemblyLine(position));
        jumps.insert(position);
    }

    void HALT(){
        assemblyCode.push_back(new HALTAssemblyLine());
    }
};