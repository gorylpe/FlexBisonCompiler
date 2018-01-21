#pragma once

#include <sstream>
#include "Command.h"

class Program {
public:
    CommandsBlock* block;

    void setBlock(CommandsBlock* block){
        this->block = block;
    }

    void ASTOptimizations(){
        block->replaceUnusedCommands();
        //ConstPropagation();
        //block->replaceUnusedCommands();
        FORUnrolling();
        RemovingIfsWithConstComparisions();
        //ConstPropagation();
        //block->replaceUnusedCommands();
        RemoveUnusedAssignements();
        OptimizeNumbers();
    }

    void ConstPropagation(){
        bool hasPropagationInLastIteration;
        do{
            hasPropagationInLastIteration = block->propagateConstants();
        }while(hasPropagationInLastIteration);
    }

    void RemoveUnusedAssignements(){

    }

    void FORUnrolling(){

    }

    void RemovingIfsWithConstComparisions(){

    }

    //memory manager
    void OptimizeNumbers(){

    }

    //todo move this to machine context
    //todo optimizations for high level instructions

    string generateCode(){
        this->ASTOptimizations();

        //after removing unused variables

        this->block->calculateVariablesUsage(0);

        memory.optimize();

        this->block->generateCode();

        machine.HALT();

        stringstream ss;
        machine.generateCode(ss);

        cerr << "End generating PROGRAM" << endl;

        return ss.str();
    }
};
