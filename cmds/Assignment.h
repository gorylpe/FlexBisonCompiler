#pragma once

#include "../Variable.h"
#include "../Command.h"
#include "AssignmentsStats.h"

class Assignment : public Command {
public:
    Identifier* ident;
    Expression* expr;

    bool unused;

    explicit Assignment(Identifier* ident, Expression* expr)
    :ident(ident)
    ,expr(expr)
    ,unused(false){
        cerr << "Creating ASSIGNMENT " << toString() << endl;
    }

    Assignment(const Assignment& asg2)
    :ident(asg2.ident->clone())
    ,expr(asg2.expr->clone())
    ,unused(false){}

    Assignment* clone() const final {
        return new Assignment(*this);
    }

    void semanticAnalysis() final {
        ident->semanticAnalysisSet();
        expr->semanticAnalysis();
    }

    void generateCode() final {
        cerr << "Generating ASSIGNMENT " << toString() << endl;
        this->ident->prepareIfNeeded();
        this->expr->prepareValuesIfNeeded();

        this->expr->loadToAccumulator();
        this->ident->storeFromAccumulator();

        this->ident->unprepareIfNeeded();
        this->expr->unprepareValuesIfNeeded();
    }

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }

        cerr << toString() << endl;
    }

    string toString() final {
        return ident->toString() + " := " + expr->toString() + ";";
    }

    bool equals(Command* command) final {
        auto assgn2 = dynamic_cast<Assignment*>(command);
        if(assgn2 == nullptr)
            return false;

        if(!this->ident->equals(assgn2->ident))
            return false;

        if(!this->expr->equals(assgn2->expr))
            return false;

        return true;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->ident->calculateVariablesUsage(numberOfNestedLoops);
        this->expr->calculateVariablesUsage(numberOfNestedLoops);
    }

    bool propagateConstants() final {
        bool hasPropagated = false;

        if(ident->propagateConstantsInPidpid()){
            hasPropagated = true;
        }

        if(expr->propagateConstants()){
            hasPropagated = true;
        }

        if(expr->isResultConst()){
            ident->setConstant(expr->getConstValue());
        } else {
            ident->unsetConstant();
        }

        return hasPropagated;
    }

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        Variable* var = ident->getPidVariable();
        if(var->type == Variable::Type::PID)
            variableSet.insert(var);
    }

    CommandsBlock* blockToReplaceWith() final {
        if(expr->equals(ident)){
            return new CommandsBlock();
        }

        if(unused){
            return new CommandsBlock();
        }

        return nullptr;
    }

    void replaceValuesWithConst(string pid, cl_I number) final {
        ident->replaceValuesWithConst(pid, number);
        expr->replaceValuesWithConst(pid, number);
    }

    void getPidsBeingUsed(set<string> &pidsSet) final {
        auto identifiers = this->expr->getIdentifiers();
        for(auto ident : identifiers){
            pidsSet.insert(ident->pid);
            if(ident->type == Identifier::Type::PIDPID){
                pidsSet.insert(ident->pidpid);
            }
        }
    }

    void collectAssignmentsStats(AssignmentsStats &prevStats) final {
        prevStats.addAssignment(this, ident, expr);
    }

    void setUnused(){
        unused = true;
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        expr->collectNumberValues(stats);
    }
};
