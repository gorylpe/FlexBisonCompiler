#pragma once

#include "../IdentifiersSSA.h"
#include "../Command.h"
#include "Assignment.h"

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
        cerr << "Generating " << toString() << endl;
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

        cerr << "Generating END " << toString() << endl;
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

    bool propagateValues(IdentifiersSSA &stats) final {
        bool hasPropagated = false;

        if(Assignment::propagateValue(cond->val1, stats)) {
            hasPropagated = true;
        }

        if(Assignment::propagateValue(cond->val2, stats)) {
            hasPropagated = true;
        }

        if(block1->propagateValues(stats))
            hasPropagated = true;

        if(block2->propagateValues(stats))
            hasPropagated = true;

        return hasPropagated;
    }

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        this->block1->getPidVariablesBeingModified(variableSet);
        this->block2->getPidVariablesBeingModified(variableSet);
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

    virtual void replaceValuesWithConst(string pid, cl_I number) {
        cond->replaceValuesWithConst(pid, number);
        block1->replaceValuesWithConst(pid, number);
        block2->replaceValuesWithConst(pid, number);
    }

    void calculateSSANumbersInIdentifiers(IdentifiersSSA &prevStats) final {
        auto oldSSAs = prevStats.getSSAsCopy();

        prevStats.addUsages(cond->getIdentifiers());
        block1->calculateSSANumbersInIdentifiers(prevStats);
        block2->calculateSSANumbersInIdentifiers(prevStats);

        prevStats.mergeWithSSAs(oldSSAs);
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        cond->collectNumberValues(stats);
        block1->collectNumberValues(stats);
        block2->collectNumberValues(stats);
    }
};