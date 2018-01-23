#pragma once

#include "AssignmentsStats.h"
#include "../Command.h"

class While : public Command {
public:
    Condition* cond;
    CommandsBlock* block;

    While(Condition* cond, CommandsBlock* block)
            :cond(cond)
            ,block(block){
        cerr << "Creating " << toString() << endl;
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

    bool propagateConstants() final {
        bool hasPropagated = false;

        set<Variable*> variablesSet;

        this->block->getPidVariablesBeingModified(variablesSet);

        //edge case, check if cond will be const and false at start
        if(cond->wouldConstantPropagateToConstComparision() && !cond->testConstComparisionIfPropagate()){
            //propagate constants to condition and remove this block in next optimizer loop
            cerr << "WHILE first check optimization on " << cond->toString() << endl;
            if(this->cond->propagateConstants()) {
                hasPropagated = true;
            }
        } else {
            for(auto var : variablesSet){
                cerr << "Var used inside while " << var->pid << endl;
                var->unsetConstant();
            }

            if(this->cond->propagateConstants()) {
                hasPropagated = true;
            }

            if(this->block->propagateConstants()){
                hasPropagated = true;
            }
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


    void getPidsBeingUsed(set<string> &pidsSet) final {
        auto identifiers = this->cond->getIdentifiers();
        for(auto ident : identifiers){
            pidsSet.insert(ident->pid);
            if(ident->type == Identifier::Type::PIDPID){
                pidsSet.insert(ident->pidpid);
            }
        }
        this->block->getPidsBeingUsed(pidsSet);
    }

    void collectAssignmentsStats(AssignmentsStats &prevStats) final {
        prevStats.addCondition(cond);
        block->collectAssignmentsStats(prevStats);

        auto pidsUsedInLoop = set<string>();
        getPidsBeingUsed(pidsUsedInLoop);

        prevStats.addPidsUsedInLoops(pidsUsedInLoop);
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        cond->collectNumberValues(stats);
        block->collectNumberValues(stats);
    }
};