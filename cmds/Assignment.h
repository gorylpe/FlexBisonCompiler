#pragma once

#include <set>
#include "../Variable.h"
#include "CommandsBlock.h"

class Assignment : public Command {
public:
    Identifier* ident;
    Expression* expr;

    explicit Assignment(Identifier* ident, Expression* expr)
            :ident(ident)
            ,expr(expr){
        cerr << "Creating ASSIGNMENT " << toString() << endl;
    }

    Assignment(const Assignment& asg2)
            :ident(asg2.ident->clone())
            ,expr(asg2.expr->clone()){}

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

        return nullptr;
    }

    void replaceValuesWithConst(string pid, cl_I number) final {
        ident->replaceValuesWithConst(pid, number);
        expr->replaceValuesWithConst(pid, number);
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        expr->collectNumberValues(stats);
    }
};


class IdentifierStats{
public:
    string pid;
    int counter;
    vector<Assignment*> assignments;
    vector<int> uses;

    explicit IdentifierStats(string pid)
    :pid(pid)
    ,counter(0){}

    void addAssignment(Assignment* assignment) {
        Identifier* ident = assignment->ident;

        ident->setSSACounter(counter);

        assignments.push_back(assignment);
        uses.push_back(0);

        counter++;
    }

    void setIdentUse(Identifier *ident){
        int lastCounter = counter - 1;
        ident->setSSACounter(lastCounter);
        uses[lastCounter]++;
    }

    string toString() const {
        stringstream ss;
        for(int i = 0; i < counter; ++i){
            ss << pid << i << "   " << uses[i] << "   " << assignments[i]->toString() << endl;
        }
        return ss.str();
    }
};

class AssignmentsStats{
public:
    map<string, IdentifierStats> stats;

    IdentifierStats getStats(string pid){
        if(stats.find(pid) == stats.end()){
            stats.insert(make_pair(pid, IdentifierStats(pid)));
        }
        return stats.at(pid);
    }

    void addAssignment(Assignment* assignment) {
        Identifier* ident = assignment->ident;
        Expression* expr = assignment->expr;

        if(ident->type == Identifier::Type::PID){
            //set last counters
            const vector<Identifier*>& idents = expr->getIdentifiers();
            for(auto id : idents){
                getStats(id->pid).setIdentUse(id);
            }
            getStats(ident->pid).addAssignment(assignment);
        }
    };

    string toString() const{
        stringstream ss;
        for(auto& entry : stats){
            const IdentifierStats& identStats = entry.second;
            ss << identStats.toString();
        }
        return ss.str();
    }
};