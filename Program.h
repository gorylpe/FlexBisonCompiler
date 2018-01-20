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

    }


    void ConstPropagation(){

    }

    void RemoveUnusedAssignements(){

    }

    void FORUnrolling(){

    }

    //memory manager
    void OptimizeNumbers(){

    }

    //todo move this to machine context
    //todo optimizations for high level instructions

    string generateCode(){
        this->ASTOptimizations();

        //after removing unused variables
        memory.optimize();

        for(auto cmd : this->block->commands){
            cmd->generateCode();
        }
        machine.HALT();

        stringstream ss;
        machine.generateCode(ss);

        cerr << "End generating PROGRAM" << endl;

        return ss.str();
    }
};
