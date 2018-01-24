#pragma once

#include "../cmdParts/IdentifiersSSAHelper.h"
#include "Command.h"
#include "Assignment.h"
#include "../cmdParts/IdentifiersUsagesHelper.h"

class If : public Command{
public:
    Condition* cond;
    CommandsBlock* block;

    If(Condition* cond, CommandsBlock* block)
            :cond(cond)
            ,block(block){
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Creating " << toString() << endl;
    #endif
    }

    If(const If& if2)
            :cond(if2.cond->clone())
            ,block(if2.block->clone()){}

    If* clone() const final {
        return new If(*this);
    }

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }

        cerr << "IF " << cond->toString() << " THEN" << endl;
        block->print(nestedLevel + 1);

        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "ENDIF" << endl;
    }

    string toString() final {
        return "IF " + cond->toString();
    }

    void semanticAnalysis() final {
        cond->semanticAnalysis();
        block->semanticAnalysis();
    }

    void generateCode() final {
        #ifdef DEBUG_LOG_GENERATING_CODE
        cerr << "Generating " << toString() << endl;
        #endif
        this->cond->prepareValuesIfNeeded();

        auto jumpIfTrue = new JumpPosition();
        auto jumpIfFalse = new JumpPosition();

        this->cond->generateTestWithTrueFalseJumps(jumpIfTrue, jumpIfFalse);
        machine.setJumpPosition(jumpIfTrue);

        block->generateCode();

        machine.setJumpPosition(jumpIfFalse);

        this->cond->unprepareValuesIfNeeded();
        #ifdef DEBUG_LOG_GENERATING_CODE
        cerr << "Generating END " << toString() << endl;
        #endif
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->cond->calculateVariablesUsage(numberOfNestedLoops);
        this->block->calculateVariablesUsage(numberOfNestedLoops);
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

        stringstream ss;
        if(pflags.verbose()) {
            ss << "PROPAGATED " << toString() << "   --->   ";
        }

        if(Assignment::tryToPropagatePidpidInCondition(assgnsHelper, usagesHelper, *cond))
            propagated++;

        if(Assignment::tryToPropagateExpressionsValueToTwoValuesInCondition(assgnsHelper, usagesHelper, *cond))
            propagated++;

        if(pflags.verbose()){
            if(propagated > 0){
                ss << toString() << endl;
                cerr << ss.str();
            }
        }

        propagated += block->propagateValues(assgnsHelper, usagesHelper);

        return propagated;
    }

    CommandsBlock* blockToReplaceWith() final{
        if(cond->isComparisionConst()){
            if(cond->getComparisionConstResult()){
                return block;
            } else {
                return new CommandsBlock();
            }
        }

        return nullptr;
    }

    void replaceCommands() final {
        this->block->replaceCommands();
    }

    void replaceValuesWithConst(string pid, cl_I number) final {
        cond->replaceValuesWithConst(pid, number);
        block->replaceValuesWithConst(pid, number);
    }

    void collectSSANumbersInIdentifiers(IdentifiersSSAHelper &stats) final {
        stats.setForUsages(cond->getIdentifiers());

        auto oldSSAs = stats.getSSAsCopy();

        block->collectSSANumbersInIdentifiers(stats);

        stats.mergeWithOldSSAs(oldSSAs);
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        cond->collectNumberValues(stats);
        block->collectNumberValues(stats);
    }
};