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
        block->replaceCommands();
        constPropagation();
        RemoveUnusedAssignements();
        OptimizeNumbers();
    }

    void constPropagation(){
        bool hadPropagation;
        int numOfPropagations;
        do{
            numOfPropagations = 0;
            do{
                hadPropagation = block->propagateConstants();
                if(hadPropagation){
                    numOfPropagations++;
                }
            }while(hadPropagation);

            cerr << "Done " << numOfPropagations << " constants propagation loops" << endl;

            if(numOfPropagations > 0){
                cerr << "Replacing unused commands or optimize using constants" << endl;
                block->replaceCommands();
            }
        }while(numOfPropagations > 0);

    }

    void RemoveUnusedAssignements(){

    }

    //memory manager
    void OptimizeNumbers(){

    }

    //todo move this to machine context
    //todo optimizations for high level instructions

    string generateCode(){
        this->ASTOptimizations();

        cerr << "---OPTIMIZED CODE---" << endl;
        block->print(0);
        cerr << "---END OPTIMIZED CODE---" << endl;

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
