#pragma once

#include "../IdentifiersSSA.h"
#include "../Command.h"
#include "Assignment.h"

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
        cerr << "Generating " << toString() << endl;
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
        cerr << "Generating END " << toString() << endl;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->cond->calculateVariablesUsage(numberOfNestedLoops + 1);
        this->block->calculateVariablesUsage(numberOfNestedLoops + 1);
    }

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        this->block->getPidVariablesBeingModified(variableSet);
    }

    void simplifyExpressions() final {
        block->simplifyExpressions();
    }

    bool propagateValues(IdentifiersSSA &stats) final {
        bool hasPropagated = false;

        //TODO do this edge case, check if cond will be const and false at start

        cerr << "WHILE PROPAGATING COND VAL1" << endl;
        if(Assignment::propagateValue(cond->val1, stats)) {
            hasPropagated = true;
        }

        cerr << "WHILE PROPAGATING COND VAL2" << endl;
        if(Assignment::propagateValue(cond->val2, stats)) {
            hasPropagated = true;
        }

        cerr << "WHILE PROPAGATING BLOCK" << endl;
        if(this->block->propagateValues(stats)){
            hasPropagated = true;
        }

        return hasPropagated;
    }


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

    void calculateSSANumbersInIdentifiers(IdentifiersSSA &prevStats) final {
        auto beforeWhileSSAs = prevStats.getSSAsCopy();

        IdentifiersSSA & tmpStats = *prevStats.clone();

        tmpStats.addUsages(cond->getIdentifiers());
        block->calculateSSANumbersInIdentifiers(tmpStats);

        auto prewhileSSAs = tmpStats.getSSAsCopy();
        cerr << "PRE WHILE" << endl;
        cerr << tmpStats.toString() << endl;

        prevStats.mergeWithSSAs(prewhileSSAs);

        cerr << "MERGED WHILE WITH PREWHILE" << endl;
        cerr << prevStats.toString() << endl;

        prevStats.addUsages(cond->getIdentifiers());
        block->calculateSSANumbersInIdentifiers(prevStats);

        prevStats.mergeWithSSAs(beforeWhileSSAs);

        cerr << "AFTER WHILE" << endl;
        cerr << prevStats.toString() << endl;
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        cond->collectNumberValues(stats);
        block->collectNumberValues(stats);
    }
};