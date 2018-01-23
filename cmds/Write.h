#pragma once


#include "../Command.h"
#include "Assignment.h"

class Write : public Command {
public:
    Value* val;

    explicit Write(Value* val)
            :val(val){
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Creating " << toString() << endl;
    #endif
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

    void simplifyExpressions() final {}

    bool propagateValues(IdentifiersSSA &stats) final {
        bool hasPropagated = false;

        if(Assignment::propagateValue(val, stats)){
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

    void calculateSSANumbersInIdentifiers(IdentifiersSSA &stats) final {
        auto ident = val->getIdentifier();
        if(ident != nullptr){
            stats.addUsage(ident);
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