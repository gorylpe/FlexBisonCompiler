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

    void valuesPropagation(){
        bool propagated;

        do{
            propagated = false;

            IdentifiersSSAHelper ssas;
            block->collectSSANumbersInIdentifiers(ssas);

            IdentifiersUsagesHelper usages;
            block->collectUsagesData(usages);

            IdentifiersAssignmentsHelper assignments;
            block->collectAssignmentsForIdentifiers(assignments);

            if(pflags.verbose())
                cerr << "---PROPAGATING VALUES---" << endl;
            int propagationsDone = block->propagateValues(assignments, usages);
            if(pflags.verbose())
                cerr << "Propagated " << propagationsDone << " values" << endl;

            usages = IdentifiersUsagesHelper();
            block->collectUsagesData(usages);


            int unusedFound = block->searchUnusedAssignmentsAndSetForDeletion(usages);
            if(pflags.verbose())
                cerr << "FOUND " << unusedFound << " UNUSED ASSIGNMENTS" << endl;

            block->simplifyExpressions();
            if(pflags.verbose())
                cerr << "EXPRESSIONS SIMPLIFIED" << endl;

            block->replaceCommands();
            if(pflags.verbose())
                cerr << "REPLACED COMMANDS" << endl;

            block->simplifyExpressions();
            if(pflags.verbose())
                cerr << "EXPRESSIONS SIMPLIFIED" << endl;

            block->semanticAnalysis(); //TODO REMOVE - only checking if optimizations didnt mess up
            if(pflags.verbose())
                cerr << "SEMANTIC ANALYSIS DONE" << endl;

            if(unusedFound > 0 || propagationsDone > 0)
                propagated = true;
        } while(propagated);
    }

    void optimizeNumbers(){
        map<cl_I, NumberValueStats> stats;

        block->collectNumberValues(stats);

        if(pflags.verbose()) {
            cerr << "---NUMBERS STATS---" << endl;
        }

        for(auto& entry : stats){
            const cl_I& num = entry.first;
            const auto& stat = entry.second;

            if(pflags.verbose()){
                cerr << " -NUMBER " << num << " STATS- " << endl;
                cerr << stat.toString() << endl;
            }
        }

        for(auto& entry : stats){
            const cl_I& num = entry.first;
            const auto& stat = entry.second;

            //if loading saved number is not profitable
            if(num <= LOAD_OPS){
                continue;
            }

            const cl_I opsToLoadSingle = Number::getLoadToAccumulatorOperations(num);

            const cl_I& opsToAddSub = num * (stat.numberOfAdditions + stat.numberOfSubtractions);
            const cl_I& opsToAddSubStored = opsToLoadSingle + STORE_OPS + LOAD_OPS * (stat.numberOfAdditions + stat.numberOfSubtractions);

            if(pflags.verbose()) {
                cerr << "OPTIMIZING NUMBER " << num << endl;
                cerr << "Time needed for adding and subbing direct number:   " << opsToAddSub << endl;
                cerr << "Time needed for store and add sub using identifier: " << opsToAddSubStored << endl;
            }

            bool profitable = false;

            //can do both at once cause same ops to inc/dec (1)
            if(opsToAddSubStored < opsToAddSub){
                profitable = true;
            }

            if(profitable){
                if(pflags.verbose()){
                    cerr << "Optimization profitable!" << endl;
                    cerr << "Optimizing subtractions and additions" << endl;
                }
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
                    if(pflags.verbose())
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
        //replacing commands like for with constants
        block->replaceCommands();
        //simplify for example 1+1 to 2
        block->simplifyExpressions();
        valuesPropagation();
        optimizeNumbers();

        if(pflags.verbose()) {
            cerr << "---OPTIMIZED CODE---" << endl;
            block->print(0);
            cerr << "---END OPTIMIZED CODE---" << endl << endl;
        }

        if(pflags.optimizeMemory()){
            //calculate usages for sorting - moving most used tabs to start of memory
            block->calculateVariablesUsage(0);
            memory.optimize();
        }

        if(pflags.verbose())
            cerr << "---GENERATING ASSEMBLY CODE---" << endl;
        this->block->generateCode();
        machine.HALT();


        machine.optimize();

        if(pflags.verbose())
            cerr << "---SAVING ASSEMBLY CODE---" << endl;
        stringstream ss;
        machine.generateCode(ss);

        return ss.str();
    }
};
