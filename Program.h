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
        cerr << "---CONST PROPAGATION AND REPLACING COMMANDS OPTIMIZATION---" << endl;
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
        cerr << "---CONST PROPAGATION AND REPLACING COMMANDS OPTIMIZATION END---" << endl << endl;
    }

    void RemoveUnusedAssignements(){

    }

    //memory manager
    void OptimizeNumbers(){

    }

    //todo move this to machine context
    //todo optimizations for high level instructions

    string generateCode(){
        block->semanticAnalysis();

        this->ASTOptimizations();

        cerr << "---OPTIMIZED CODE---" << endl;
        block->print(0);
        cerr << "---END OPTIMIZED CODE---" << endl << endl;

        //after removing unused variables

        this->block->calculateVariablesUsage(0);

        memory.optimize();

        cerr << "---GENERATING ASSEMBLY CODE---" << endl;
        this->block->generateCode();
        machine.HALT();
        cerr << "---GENERATING ASSEMBLY CODE END---" << endl << endl;

        machine.optimize();

        cerr << "---SAVING ASSEMBLY CODE---" << endl;
        stringstream ss;
        machine.generateCode(ss);

        return ss.str();
    }
};
