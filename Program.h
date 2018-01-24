#pragma once

#include <sstream>
#include "Command.h"
#include "cmds/Assignment.h"

class Program {
public:
    CommandsBlock* block;

    void setBlock(CommandsBlock* block){
        this->block = block;
    }

    void ASTOptimizations(){
        block->replaceCommands();
        block->simplifyExpressions();
        valuesPropagation();
        optimizeNumbers();
    }

    void valuesPropagation(){
        bool propagated;

        do{
            propagated = false;

            IdentifiersSSAHelper stats;
            cerr << "!!!STATS!!!" << endl;
            block->collectSSANumbersInIdentifiers(stats);
            cerr << stats.toString() << endl;

            block->print(0);

            //collect usages data after propagation
            IdentifiersUsagesHelper usages;
            cerr << "!!!USAGES!!!" << endl;
            block->collectUsagesData(usages);
            cerr << usages.toString() << endl;

            IdentifiersAssignmentsHelper assignments;
            block->collectAssignmentsForIdentifiers(assignments);

            for(auto& entry : assignments.assignments){
                cerr << "Collected " << entry.second->assignments.size() << " assignments for " << entry.first << endl;
            }

            cerr << "PROPAGATING" << endl;
            int propagationsDone = block->propagateValues(assignments, usages);
            cerr << "Propagated " << propagationsDone << " values" << endl;

            block->print(0);

            cerr << "!!!USAGES AFTER PROPAGATION!!!" << endl;
            usages = IdentifiersUsagesHelper();
            block->collectUsagesData(usages);
            cerr << usages.toString() << endl;

            int unusedFound = block->searchUnusedAssignmentsAndSetForDeletion(usages);
            cerr << "FOUND " << unusedFound << " UNUSED ASSIGNMENTS" << endl;

            block->simplifyExpressions();
            cerr << "EXPRESSIONS SIMPLIFIED" << endl;

            block->replaceCommands();
            cerr << "REPLACED COMMANDS" << endl;

            block->simplifyExpressions();
            cerr << "EXPRESSIONS SIMPLIFIED" << endl;

            block->semanticAnalysis(); //TODO REMOVE - only checking if optimizations didnt mess up
            cerr << "SEMANTIC ANALYSIS DONE" << endl;

            if(unusedFound > 0 || propagationsDone > 0)
                propagated = true;
        } while(propagated);
    }

    void optimizeNumbers(){
        map<cl_I, NumberValueStats> stats;

        block->collectNumberValues(stats);

        for(auto& entry : stats){
            const cl_I& num = entry.first;
            const auto& stat = entry.second;

            cerr << "---NUMBER " << num << " ---" << endl;
            cerr << stat.toString() << endl;
        }

        for(auto& entry : stats){
            const cl_I& num = entry.first;
            const auto& stat = entry.second;

            //if loading saved number is not profitable
            if(num <= LOAD_OPS){
                continue;
            }

            cerr << "Optimizing " << num << endl;

            const cl_I opsToLoadSingle = Number::getLoadToAccumulatorOperations(num);

            const cl_I& opsToAddSub = num * (stat.numberOfAdditions + stat.numberOfSubtractions);
            const cl_I& opsToAddSubStored = opsToLoadSingle + STORE_OPS + LOAD_OPS * (stat.numberOfAdditions + stat.numberOfSubtractions);

            cerr << "Load ops              " << opsToLoadSingle << endl;

            cerr << "Ops to add+sub stored " << opsToAddSubStored << endl;
            cerr << "Ops to add+sub        " << opsToAddSub << endl;

            bool profitable = false;

            //can do both at once cause same ops to inc/dec (1)
            if(opsToAddSubStored < opsToAddSub){
                profitable = true;
            }

            if(profitable){
                cerr << "Optimizing subtractions and additions" << endl;
                string pid = memory.addNumberVariable();

                auto number = new Number(nullptr, num);
                number->loadToAccumulator();
                delete number;

                Identifier ident(new Position(0,0,0,0), pid);
                ident.storeFromAccumulator();

                for(auto valSub : stat.valsSubtracted){
                    valSub->setIdentifier(new Identifier(ident));
                }

                for(auto valAdd : stat.valsAdded){
                    valAdd->setIdentifier(new Identifier(ident));
                }

                if(LOAD_OPS < opsToLoadSingle){
                    cerr << "Optimizing loads" << endl;
                    for(auto valLoad : stat.valsLoaded){
                        valLoad->setIdentifier(new Identifier(ident));
                    }
                }
            }
        }

    }

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
