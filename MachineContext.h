#pragma once

#include <set>
#include <algorithm>
#include "MemoryManager.h"
#include "Assembly.h"
#include "ProgramFlags.h"

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

    void optimize(){
        if(pflags.verbose())
            cerr << endl << "---OPTIMIZING ASSEMBLY CODE---" << endl << endl;
        optimizeRedundandLoadsAfterStoreInContinuousCodeBlocks();
        optimizeRedundandStoresInContinuousCodeBlocks();
        optimizeRedundandIncDecInContinuousCodeBlocks();
    }

    void generateCode(stringstream& ss){
        for(auto line : assemblyCode){
            line->toStringstream(ss);
        }
    }

    void optimizeRedundandLoadsAfterStoreInContinuousCodeBlocks() {
        if(pflags.verbose())
            cerr << "---REDUNDANT LOADS AFTER STORE OPTIMIZATION---" << endl;
        size_t linesNum = (size_t)assemblyCode.size();

        vector<bool> linesWithJump = this->getLinesWithJump();

        vector<bool> linesToRemove(linesNum);
        std::fill(linesToRemove.begin(), linesToRemove.end(), false);

        //OPTIMIZE STORE LOADS
        vector<cl_I> currMemoryPtrs;
        for (int i = 0; i < linesNum; ++i) {
            if (linesWithJump[i]) {
                //reset current accumulator memory ptr
                currMemoryPtrs.clear();
            }
            auto storeLine = dynamic_cast<STOREAssemblyLine *>(assemblyCode[i]);
            auto storeILine = dynamic_cast<STOREAssemblyLine *>(assemblyCode[i]);
            auto loadLine = dynamic_cast<LOADAssemblyLine *>(assemblyCode[i]);
            auto putLine = dynamic_cast<PUTAssemblyLine *>(assemblyCode[i]);

            if (storeLine != nullptr) {
                //add new address to current memory pointers
                currMemoryPtrs.push_back(storeLine->getMemoryPtr());
            } else if (loadLine != nullptr) {
                //check if current load line equals current accumulator
                if (find(currMemoryPtrs.begin(), currMemoryPtrs.end(), loadLine->getMemoryPtr()) != currMemoryPtrs.end()){
                    linesToRemove[i] = true;
                } else {
                    currMemoryPtrs.clear();
                    currMemoryPtrs.push_back(loadLine->getMemoryPtr());
                }
            } else if (storeILine == nullptr && putLine == nullptr){
                //if current line is one with influence on accumulator
                currMemoryPtrs.clear();
            }
        }

        this->removeLines(linesToRemove);

        if(pflags.verbose())
            cerr << "---REDUNDANT LOADS AFTER STORE OPTIMIZATION END---" << endl << endl;
    }

    void optimizeRedundandStoresInContinuousCodeBlocks() {
        if(pflags.verbose())
            cerr << "---REDUNDANT STORES OPTIMIZATION---" << endl;
        size_t linesNum = (size_t)assemblyCode.size();

        vector<bool> linesWithJump = this->getLinesWithJump();

        vector<bool> linesToRemove(linesNum);
        std::fill(linesToRemove.begin(), linesToRemove.end(), false);

        //OPTIMIZE STORE LOADS
        cl_I lastStorePtr = -1;
        int lastStoreLineNumber = -1;
        for (int i = 0; i < linesNum; ++i) {
            if (linesWithJump[i]) {
                //reset last store line
                lastStoreLineNumber = -1;
                lastStorePtr = -1;
            }
            auto storeLine = dynamic_cast<STOREAssemblyLine *>(assemblyCode[i]);
            auto storeILine = dynamic_cast<STOREIAssemblyLine *>(assemblyCode[i]);
            auto loadLine = dynamic_cast<LOADAssemblyLine *>(assemblyCode[i]);
            auto loadILine = dynamic_cast<LOADIAssemblyLine *>(assemblyCode[i]);
            auto addLine = dynamic_cast<ADDAssemblyLine *>(assemblyCode[i]);
            auto subLine = dynamic_cast<SUBAssemblyLine *>(assemblyCode[i]);

            if (storeLine != nullptr) {
                //add new address to current memory pointers
                if(lastStorePtr == storeLine->getMemoryPtr()){
                    linesToRemove[lastStoreLineNumber] = true;
                }
                lastStoreLineNumber = i;
                lastStorePtr = storeLine->getMemoryPtr();
            } else if (loadLine != nullptr) {
                //check if current line equals last store and reset if it is
                if(lastStorePtr == loadLine->getMemoryPtr()){
                    lastStoreLineNumber = -1;
                    lastStorePtr = -1;
                }
            } else if (addLine != nullptr) {
                //check if current line equals last store and reset if it is
                if(lastStorePtr == addLine->getMemoryPtr()){
                    lastStoreLineNumber = -1;
                    lastStorePtr = -1;
                }
            } else if (subLine != nullptr) {
                //check if current line equals last store and reset if it is
                if(lastStorePtr == subLine->getMemoryPtr()){
                    lastStoreLineNumber = -1;
                    lastStorePtr = -1;
                }
            } else if (storeILine != nullptr) {
                //check if current line equals last store and reset if it is
                if(lastStorePtr == storeILine->getMemoryPtr()){
                    lastStoreLineNumber = -1;
                    lastStorePtr = -1;
                }
            } else if (loadILine != nullptr) {
                //check if current line equals last store and reset if it is
                if(lastStorePtr == loadILine->getMemoryPtr()){
                    lastStoreLineNumber = -1;
                    lastStorePtr = -1;
                }
            }
        }

        this->removeLines(linesToRemove);

        if(pflags.verbose())
            cerr << "---REDUNDANT STORES OPTIMIZATION END---" << endl << endl;
    }

    void optimizeRedundandIncDecInContinuousCodeBlocks() {
        if(pflags.verbose())
            cerr << "---REDUNDANT INC DEC OPTIMIZATION---" << endl;
        size_t linesNum = (size_t)assemblyCode.size();

        vector<bool> linesWithJump = this->getLinesWithJump();

        vector<bool> linesToRemove(linesNum);
        std::fill(linesToRemove.begin(), linesToRemove.end(), false);

        //OPTIMIZE INC DEC
        bool lastWasInc = false;
        int lastStoreLineNumber = -1;
        for (int i = 0; i < linesNum; ++i) {
            if (linesWithJump[i]) {
                lastWasInc = false;
            }
            auto incLine = dynamic_cast<INCAssemblyLine *>(assemblyCode[i]);
            auto decLine = dynamic_cast<DECAssemblyLine *>(assemblyCode[i]);

            if (incLine != nullptr) {
                lastWasInc = true;
            } else if (decLine != nullptr) {
                if(lastWasInc){
                    linesToRemove[i] = true;
                    linesToRemove[i-1] = true;
                }
                lastWasInc = false;
            } else {
                lastWasInc = false;
            }
        }

        this->removeLines(linesToRemove);

        if(pflags.verbose())
            cerr << "---REDUNDANT INC DEC OPTIMIZATION END---" << endl << endl;
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
                if(pflags.verbose())
                    cerr << "Removing assembly line no. " << i << endl;
                assemblyCode.erase(assemblyCode.begin() + i);
            }
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