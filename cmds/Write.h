#pragma once


#include "../Command.h"

class Write : public Command {
public:
    Value* val;

    explicit Write(Value* val)
            :val(val){
        cerr << "Creating " << toString() << endl;
    }

    Write(const Write& read2)
            :val(read2.val->clone()){}

    Write* clone() const final {
        return new Write(*this);
    }

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }

        cerr << toString() << endl;
    }

    string toString() final {
        return "WRITE " + val->toString() + ";";
    }

    void semanticAnalysis() final {
        val->semanticAnalysis();
    }

    void generateCode() final{
        cerr << "Generating " << toString() << endl;
        if(this->val->type != Value::Type::NUM)
            this->val->prepareIfNeeded();

        this->val->loadToAccumulator();
        machine.PUT();

        if(this->val->type != Value::Type::NUM)
            this->val->unprepareIfNeeded();
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->val->calculateVariablesUsage(numberOfNestedLoops);
    }

    bool propagateConstants() final {
        bool hasPropagated = false;

        if(val->propagateConstant()){
            hasPropagated = true;
        }

        return hasPropagated;
    }

    CommandsBlock* blockToReplaceWith(){
        return nullptr;
    }

    virtual void replaceValuesWithConst(string pid, cl_I number) {
        val->replaceIdentifierWithConst(pid, number);
    }

    void collectAssignmentsStats(AssignmentsStats &prevStats) final {
        auto ident = val->getIdentifier();
        if(ident != nullptr){
            prevStats.addUsage(ident);
        }
    }

    void getPidsBeingUsed(set<string> &pidsSet) final {
        auto ident = val->getIdentifier();
        if(ident != nullptr){
            pidsSet.insert(ident->pid);
            if(ident->type == Identifier::Type::PIDPID){
                pidsSet.insert(ident->pidpid);
            }
        }
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        if(val->type == Value::Type::NUM){
            if(stats.count(val->num->num) == 0){
                stats[val->num->num] = NumberValueStats();
            }
            stats[val->num->num].addLoad(val, 1);
        }
    }
};