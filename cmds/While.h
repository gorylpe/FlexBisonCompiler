#pragma once

#include "../cmdParts/IdentifiersSSAHelper.h"
#include "Command.h"
#include "Assignment.h"
#include "../cmdParts/IdentifiersUsagesHelper.h"

class While : public Command {
public:
    Condition* cond;
    CommandsBlock* block;

    While(Condition* cond, CommandsBlock* block)
            :cond(cond)
            ,block(block){
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Creating " << toString() << endl;
    #endif
    }

    While(const While& while2)
            :cond(while2.cond->clone())
            ,block(while2.block->clone()){}

    While* clone() const final {
        return new While(*this);
    }

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "WHILE " << cond->toString() << " DO" << endl;

        block->print(nestedLevel + 1);

        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "ENDWHILE " << endl;
    }

    string toString() final {
        return "WHILE " + cond->toString();
    }

    void semanticAnalysis() final {
        cond->semanticAnalysis();
        block->semanticAnalysis();
    }

    void generateCode() final {
        #ifdef DEBUG_LOG_GENERATING_CODE
        cerr << "Generating " << toString() << endl;
        #endif
        if(!this->cond->isComparisionConst()){
            this->cond->prepareValuesIfNeeded();

            auto loopStart = new JumpPosition();
            auto codeStart = new JumpPosition();
            auto loopOutside = new JumpPosition();

            machine.setJumpPosition(loopStart);

            this->cond->generateTestWithTrueFalseJumps(codeStart, loopOutside);

            machine.setJumpPosition(codeStart);

            block->generateCode();

            machine.JUMP(loopStart);

            machine.setJumpPosition(loopOutside);

            this->cond->unprepareValuesIfNeeded();
        } else if (this->cond->getComparisionConstResult()) {
            auto loopStart = new JumpPosition();
            machine.setJumpPosition(loopStart);

            block->generateCode();

            machine.JUMP(loopStart);
        }
        #ifdef DEBUG_LOG_GENERATING_CODE
        cerr << "Generating END " << toString() << endl;
        #endif
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->cond->calculateVariablesUsage(numberOfNestedLoops + 1);
        this->block->calculateVariablesUsage(numberOfNestedLoops + 1);
    }

    void simplifyExpressions() final {
        block->simplifyExpressions();
    }

    void collectUsagesData(IdentifiersUsagesHelper &helper) final {
        helper.addUsages(cond->getIdentifiers());
        block->collectUsagesData(helper);
    }

    int searchUnusedAssignmentsAndSetForDeletion(IdentifiersUsagesHelper &helper) final {
        return block->searchUnusedAssignmentsAndSetForDeletion(helper);
    }

    void collectAssignmentsForIdentifiers(IdentifiersAssignmentsHelper& helper) final {
        block->collectAssignmentsForIdentifiers(helper);
    }

    int propagateValues(IdentifiersAssignmentsHelper &assgnsHelper, IdentifiersUsagesHelper &usagesHelper) final {
        int propagated = 0;

        if(Assignment::tryToPropagatePidpidInCondition(assgnsHelper, usagesHelper, *cond))
            propagated++;

        if(Assignment::tryToPropagateExpressionsValueToTwoValuesInCondition(assgnsHelper, usagesHelper, *cond))
            propagated++;

        propagated += block->propagateValues(assgnsHelper, usagesHelper);
        return propagated;
    }

    //TODO do this edge case, check if cond will be const and false at start

    CommandsBlock* blockToReplaceWith() final{
        if(cond->isComparisionConst()){
            if(!cond->getComparisionConstResult()){
                return new CommandsBlock();
            }
        }

        return nullptr;
    }

    void replaceCommands() final {
        this->block->replaceCommands();
    }

    virtual void replaceValuesWithConst(string pid, cl_I number) {
        cond->replaceValuesWithConst(pid, number);
        block->replaceValuesWithConst(pid, number);
    }

    void collectSSANumbersInIdentifiers(IdentifiersSSAHelper &prevStats) final {
        auto beforeWhileSSAs = prevStats.getSSAsCopy();

        IdentifiersSSAHelper & tmpStats = *prevStats.clone();

        tmpStats.setForUsages(cond->getIdentifiers());
        block->collectSSANumbersInIdentifiers(tmpStats);

        auto prewhileSSAs = tmpStats.getSSAsCopy();

        prevStats.mergeWithOldSSAs(prewhileSSAs);

        prevStats.setForUsages(cond->getIdentifiers());
        block->collectSSANumbersInIdentifiers(prevStats);

        prevStats.mergeWithOldSSAs(beforeWhileSSAs);
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        cond->collectNumberValues(stats);
        block->collectNumberValues(stats);
    }
};