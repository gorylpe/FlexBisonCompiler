#pragma once


#include "../Command.h"
#include "Assignment.h"
#include "../IdentifiersUsagesHelper.h"

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
        #ifdef DEBUG_LOG_GENERATING_CODE
        cerr << "Generating " << toString() << endl;
        #endif
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

    void collectUsagesData(IdentifiersUsagesHelper &helper) final {
        if(val->isTypeIDENTIFIER()){
            helper.addUsage(val->getIdentifier());
        }
    }

    int searchUnusedAssignmentsAndSetForDeletion(IdentifiersUsagesHelper &helper) final { return 0; }

    void collectAssignmentsForIdentifiers(IdentifiersAssignmentsHelper& helper) final {}

    int propagateValues(IdentifiersAssignmentsHelper &assgnsHelper, IdentifiersUsagesHelper &usagesHelper) final {
        int propagated = 0;

        stringstream ss;
        if(pflags.verbose())
            ss << "PROPAGATED " << toString() << "   --->   ";

        if(val->isTypeIDENTIFIER() && val->ident->isTypePIDPID()){
            if(Assignment::tryToPropagatePidpid(assgnsHelper, usagesHelper, *val->ident)){
                ++propagated;
            }
        }

        if(val->isTypeIDENTIFIER() && val->ident->isTypePID()){
            Expression* prevExpr = Assignment::getExpressionAssignedToValueWithOneUsage(assgnsHelper, usagesHelper, *val);

            if(prevExpr != nullptr && prevExpr->isTypeVALUE()){
                val = prevExpr->val1->clone();
                ++propagated;
            }
        }

        if(pflags.verbose()){
            if(propagated > 0){
                ss << toString() << endl;
                cerr << ss.str();
            }
        }

        return propagated;
    }

    CommandsBlock* blockToReplaceWith() final {
        return nullptr;
    }

    void replaceValuesWithConst(string pid, cl_I number) final {
        val->replaceIdentifierWithConst(pid, number);
    }

    void collectSSANumbersInIdentifiers(IdentifiersSSAHelper &stats) final {
        if(val->isTypeIDENTIFIER()){
            stats.setForUsage(val->getIdentifier());
        }
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        if(val->isTypeNUM()){
            if(stats.count(val->num->num) == 0){
                stats[val->num->num] = NumberValueStats();
            }
            stats[val->num->num].addLoad(val, 1);
        }
    }
};