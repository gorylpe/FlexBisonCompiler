#pragma once

#include <set>
#include <algorithm>
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

    //todo constants propagation in AST - 9-sort test, 5-tab, 1-numbers, program2

    //todo CHECK FOR UNUSED VARS (not likely)

    //todo optimize STOREs, if there's 2 assignments

    //todo optimize INC DEC

    //todo if we have assignment -> storing val and next one is loading this val and HAS JUMP TO THIS INSTRUCTION
    //(something else is using this LOAD)
    //then GENERATE JUMP TO NEXT INSTRUCTION -> easier to jump than load

    //todo UNROLLING FOR WITH CONSTANS - VERY BIG OPTIMIZATIONS

    //todo code generation, look for STORE LOAD duplications, JUMP to line after
    void optimizeRedundandLoadsAfterStoreInContinuousCodeBlocks() {
        cerr << "---REDUNDANT LOADS AFTER STORE OPTIMIZATION---" << endl;
        size_t linesNum = (size_t)assemblyCode.size();

        vector<bool> linesWithJump = this->getLinesWithJump();

        vector<bool> linesToRemove(linesNum);
        std::fill(linesToRemove.begin(), linesToRemove.end(), false);

        //OPTIMIZE STORE LOADS
        cl_I currentAccumulatorMemoryPtr = -1;
        for (int i = 0; i < linesNum; ++i) {
            if (linesWithJump[i]) {
                //reset current accumulator memory ptr
                currentAccumulatorMemoryPtr = -1;
            }
            auto storeLine = dynamic_cast<STOREAssemblyLine *>(assemblyCode[i]);
            auto loadLine = dynamic_cast<LOADAssemblyLine *>(assemblyCode[i]);

            if (storeLine != nullptr) {
                currentAccumulatorMemoryPtr = storeLine->getMemoryPtr();
            } else if (loadLine != nullptr) {
                if (loadLine->getMemoryPtr() == currentAccumulatorMemoryPtr && !linesWithJump[i]) {
                    linesToRemove[i] = true;
                }
                currentAccumulatorMemoryPtr = loadLine->getMemoryPtr();
            }
        }

        this->removeLines(linesToRemove);

        cerr << "---OPTIMIZATION END---" << endl;
    }

    vector<bool> getLinesWithJump(){
        int linesNum = (int)assemblyCode.size();

        vector<bool> linesWithJump(linesNum);
        for (int i = 0; i < linesNum; ++i) {
            linesWithJump[i] = dynamic_cast<JumpAssemblyLine *>(assemblyCode[i]) != nullptr;
        }

        for (auto jump : jumps) {
            linesWithJump[jump->getPosition()] = true;
        }

        return linesWithJump;
    }

    void removeLines(vector<bool>& linesToRemove){
        int linesNum = (int)assemblyCode.size();

        //calculate jump shifts
        vector<int> jumpShift(linesNum);
        jumpShift[0] = 0;
        for (int i = 1; i < linesNum; ++i) {
            jumpShift[i] = jumpShift[i - 1] + (linesToRemove[i] ? true : false);
        }

        //shift jumps
        for (auto jump : jumps) {
            unsigned long position = jump->getPosition();
            position -= jumpShift[position];
            jump->setPosition(position);
        }

        //remove unused lines
        for(int i = linesNum - 1; i >= 0; --i){
            if(linesToRemove[i]){
                cerr << "Removing assembly line no. " << i << endl;
                assemblyCode.erase(assemblyCode.begin() + i);
            }
        }
    }


    void generateCode(stringstream& ss){
        this->optimizeRedundandLoadsAfterStoreInContinuousCodeBlocks();
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