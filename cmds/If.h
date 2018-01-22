#pragma once

#include "../Command.h"

class If : public Command{
public:
    Condition* cond;
    CommandsBlock* block;

    If(Condition* cond, CommandsBlock* block)
            :cond(cond)
            ,block(block){
        cerr << "Creating " << toString() << endl;
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
        cerr << "Generating " << toString() << endl;
        this->cond->prepareValuesIfNeeded();

        auto jumpIfTrue = new JumpPosition();
        auto jumpIfFalse = new JumpPosition();

        this->cond->generateTestWithTrueFalseJumps(jumpIfTrue, jumpIfFalse);
        machine.setJumpPosition(jumpIfTrue);

        block->generateCode();

        machine.setJumpPosition(jumpIfFalse);

        this->cond->unprepareValuesIfNeeded();
        cerr << "Generating END " << toString() << endl;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->cond->calculateVariablesUsage(numberOfNestedLoops);
        this->block->calculateVariablesUsage(numberOfNestedLoops);
    }

    bool propagateConstants() final {
        bool hasPropagated = false;

        if(cond->propagateConstants())
            hasPropagated = true;

        if(block->propagateConstants())
            hasPropagated = true;

        return hasPropagated;
    }

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        this->block->getPidVariablesBeingModified(variableSet);
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

    virtual void replaceValuesWithConst(string pid, cl_I number) {
        cond->replaceValuesWithConst(pid, number);
        block->replaceValuesWithConst(pid, number);
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        cond->collectNumberValues(stats);
        block->collectNumberValues(stats);
    }
};