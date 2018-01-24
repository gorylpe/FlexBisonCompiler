#pragma once

#include "../IdentifiersSSAHelper.h"
#include "../Command.h"
#include "Assignment.h"
#include "../IdentifiersUsagesHelper.h"

class IfElse : public Command{
public:
    Condition* cond;
    CommandsBlock* block1;
    CommandsBlock* block2;

    IfElse(Condition* cond, CommandsBlock* block1, CommandsBlock* block2)
            :cond(cond)
            ,block1(block1)
            ,block2(block2){
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Creating " << toString() << endl;
    #endif
    }

    IfElse(const IfElse& if2)
            :cond(if2.cond->clone())
            ,block1(if2.block1->clone())
            ,block2(if2.block2->clone()){}

    IfElse* clone() const final {
        return new IfElse(*this);
    }

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "IF " << cond->toString() << " THEN" << endl;

        block1->print(nestedLevel + 1);

        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "ELSE " << endl;

        block2->print(nestedLevel + 1);

        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "ENDIF" << endl;
    }

    string toString() final {
        return "IF " + cond->toString() + " ELSE";
    }

    void semanticAnalysis() final {
        cond->semanticAnalysis();
        block1->semanticAnalysis();
        block2->semanticAnalysis();
    }

    void generateCode() final {
        #ifdef DEBUG_LOG_GENERATING_CODE
        cerr << "Generating " << toString() << endl;
        #endif
        this->cond->prepareValuesIfNeeded();

        auto jumpIfTrue = new JumpPosition();
        auto jumpIfFalse = new JumpPosition();
        auto passElseBlock = new JumpPosition();

        this->cond->generateTestWithTrueFalseJumps(jumpIfTrue, jumpIfFalse); //jump to else

        machine.setJumpPosition(jumpIfTrue);

        block1->generateCode();

        machine.JUMP(passElseBlock);

        machine.setJumpPosition(jumpIfFalse);

        block2->generateCode();

        machine.setJumpPosition(passElseBlock);

        this->cond->unprepareValuesIfNeeded();
        #ifdef DEBUG_LOG_GENERATING_CODE
        cerr << "Generating END " << toString() << endl;
        #endif
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->cond->calculateVariablesUsage(numberOfNestedLoops);
        this->block1->calculateVariablesUsage(numberOfNestedLoops);
        this->block2->calculateVariablesUsage(numberOfNestedLoops);
    }

    void simplifyExpressions() final {
        block1->simplifyExpressions();
        block2->simplifyExpressions();
    }

    void collectUsagesData(IdentifiersUsagesHelper &helper) final {
        helper.addUsages(cond->getIdentifiers());
        block1->collectUsagesData(helper);
        block2->collectUsagesData(helper);
    }

    int searchUnusedAssignmentsAndSetForDeletion(IdentifiersUsagesHelper &helper) final {
        int removed = 0;
        removed += block1->searchUnusedAssignmentsAndSetForDeletion(helper);
        removed += block2->searchUnusedAssignmentsAndSetForDeletion(helper);
        return removed;
    }

    void collectAssignmentsForIdentifiers(IdentifiersAssignmentsHelper& helper) final {
        block1->collectAssignmentsForIdentifiers(helper);
        block2->collectAssignmentsForIdentifiers(helper);
    }

    int propagateValues(IdentifiersAssignmentsHelper &assgnsHelper, IdentifiersUsagesHelper &usagesHelper) final {
        int propagated = 0;

        stringstream ss;
        if(pflags.verbose()) {
            ss << "PROPAGATED" << endl;
            ss << toString() << endl;
            ss << " TO " << endl;
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

        propagated += block1->propagateValues(assgnsHelper, usagesHelper);
        propagated += block2->propagateValues(assgnsHelper, usagesHelper);
        return propagated;
    }

    CommandsBlock* blockToReplaceWith() final{
        if(cond->isComparisionConst()){
            if(cond->getComparisionConstResult()){
                return block1;
            } else {
                return block2;
            }
        }

        return nullptr;
    }

    void replaceCommands() final {
        this->block1->replaceCommands();
        this->block2->replaceCommands();
    }

    void replaceValuesWithConst(string pid, cl_I number) final {
        cond->replaceValuesWithConst(pid, number);
        block1->replaceValuesWithConst(pid, number);
        block2->replaceValuesWithConst(pid, number);
    }

    void collectSSANumbersInIdentifiers(IdentifiersSSAHelper &stats) final {
        stats.setForUsages(cond->getIdentifiers());

        auto oldSSAs = stats.getSSAsCopy();

        block1->collectSSANumbersInIdentifiers(stats);

        auto block1SSAs = stats.getSSAsCopy();

        stats.resetToOldSSAs(oldSSAs);

        block2->collectSSANumbersInIdentifiers(stats);

        stats.mergeWithOldSSAs(oldSSAs);
        stats.mergeWithOldSSAs(block1SSAs);
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        cond->collectNumberValues(stats);
        block1->collectNumberValues(stats);
        block2->collectNumberValues(stats);
    }
};