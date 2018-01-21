#pragma once

#include "Value.h"

class Condition {
public:
    enum Type{
        EQ = 0,
        NEQ,
        LT,
        GT,
        LEQ,
        GEQ
    };
    Type type;

    Value* val1;
    Value* val2;
    bool constComparision;
    bool constComparisionResult;

    Condition(Type type, Value* val1, Value* val2)
    :type(type)
    ,val1(val1)
    ,val2(val2)
    ,constComparision(false)
    ,constComparisionResult(false){
        //const comparision optimisation
        this->optimizeConstants();
        cerr << "Creating condition " << toString() << endl;
    }

    string toString(){
        string condType;
        switch (type){
            case EQ:
                condType = "=";
                break;
            case NEQ:
                condType = "<>";
                break;
            case LT:
                condType = "<";
                break;
            case GT:
                condType = ">";
                break;
            case LEQ:
                condType = "<=";
                break;
            case GEQ:
                condType = ">=";
                break;
        }
        return val1->toString() + " " + condType + " " + val2->toString();
    }

    bool equals(Condition* cond2){
        if(this->isComparisionConst() && cond2->isComparisionConst()){
            return this->getComparisionConstResult() == cond2->getComparisionConstResult();
        } else if(!this->isComparisionConst() && !cond2->isComparisionConst()) {
            bool isEqual = false;
            switch (type){
                case EQ:
                    if(cond2->type == EQ) {
                        if(this->val1->equals(cond2->val1) && this->val2->equals(cond2->val2))
                            isEqual = true;
                        if(this->val2->equals(cond2->val1) && this->val1->equals(cond2->val2))
                            isEqual = true;
                    }
                    break;
                case NEQ:
                    if(cond2->type == NEQ) {
                        if(this->val1->equals(cond2->val1) && this->val2->equals(cond2->val2))
                            isEqual = true;
                        if(this->val2->equals(cond2->val1) && this->val1->equals(cond2->val2))
                            isEqual = true;
                    }
                    break;
                case LT:
                    if(cond2->type == LT) {
                        if(this->val1->equals(cond2->val1) && this->val2->equals(cond2->val2))
                            isEqual = true;
                    } else if(cond2->type == GT){
                        if(this->val2->equals(cond2->val1) && this->val1->equals(cond2->val2))
                            isEqual = true;
                    }
                    break;
                case GT:
                    if(cond2->type == GT) {
                        if(this->val1->equals(cond2->val1) && this->val2->equals(cond2->val2))
                            isEqual = true;
                    } else if(cond2->type == LT){
                        if(this->val2->equals(cond2->val1) && this->val1->equals(cond2->val2))
                            isEqual = true;
                    }
                    break;
                case LEQ:
                    if(cond2->type == LEQ) {
                        if(this->val1->equals(cond2->val1) && this->val2->equals(cond2->val2))
                            isEqual = true;
                    } else if(cond2->type == GEQ){
                        if(this->val2->equals(cond2->val1) && this->val1->equals(cond2->val2))
                            isEqual = true;
                    }
                    break;
                case GEQ:
                    if(cond2->type == GEQ) {
                        if(this->val1->equals(cond2->val1) && this->val2->equals(cond2->val2))
                            isEqual = true;
                    } else if(cond2->type == LEQ){
                        if(this->val2->equals(cond2->val1) && this->val1->equals(cond2->val2))
                            isEqual = true;
                    }
                    break;
            }
            return isEqual;
        }
        return false;
    }

    void semanticAnalysis(){
        val1->semanticAnalysis();
        val2->semanticAnalysis();
    }

    void prepareValuesIfNeeded(){
        this->val1->prepareIfNeeded();
        this->val2->prepareIfNeeded();
    }

    void unprepareValuesIfNeeded(){
        this->val1->unprepareIfNeeded();
        this->val2->unprepareIfNeeded();
    }


    void generateTestWithTrueFalseJumps(JumpPosition *jumpIfTrue, JumpPosition *jumpIfFalse) {
        switch (this->type) {
            case EQ:
                this->generateTestEquals(jumpIfTrue, jumpIfFalse);
                break;
            case NEQ:
                this->generateTestNotEquals(jumpIfTrue, jumpIfFalse);
                break;
            case LT:
                val2->loadToAccumulator();
                val1->subFromAccumulator();
                machine.JZERO(jumpIfFalse); //b <= a; jump outside
                // b > a
                break;
            case GT:
                val1->loadToAccumulator();
                val2->subFromAccumulator();
                machine.JZERO(jumpIfFalse);// a <= b; jump outside commands
                // a > b;
                break;
            case LEQ:
                val1->loadToAccumulator();
                val2->subFromAccumulator();
                machine.JZERO(jumpIfTrue); // a <= b; jump to instructions
                machine.JUMP(jumpIfFalse); // a > b; jump outside instructions
                break;
            case GEQ:
                val2->loadToAccumulator();
                val1->subFromAccumulator();
                machine.JZERO(jumpIfTrue); // a <= b; jump to instructions
                machine.JUMP(jumpIfFalse); // a > b; jump outside instructions
                break;
        }
    }

    void generateTestNotEquals(JumpPosition *jumpIfTrue, JumpPosition *jumpIfFalse){
        if(val1->type == Value::Type::NUM || val2->type == Value::Type::NUM){
            this->generateTestNotEqualsForNumber(jumpIfTrue, jumpIfFalse);
        } else {
            this->generateTestNotEqualsDefault(jumpIfTrue, jumpIfFalse);
        }
    }

    bool isSmall(Value* valNum){
        return valNum->num->num < 3;
    }

    void generateTestNotEqualsForNumber(JumpPosition *jumpIfTrue, JumpPosition *jumpIfFalse){
        Value* valNum = val1->type == Value::Type::NUM ? val1 : val2;
        Value* valNotNum = val1->type == Value::Type::NUM ? val2 : val1;

        //optimization for comparing with small numbers tested empirically
        if(isSmall(valNum)){
            valNotNum->loadToAccumulator();
            //jump to false if already zero
            for(cl_I i = 0; i < valNum->num->num; ++i){
                machine.JZERO(jumpIfTrue);
                machine.DEC();
            }
            machine.JZERO(jumpIfFalse);
        } else {
            this->generateTestNotEqualsDefault(jumpIfTrue, jumpIfFalse);
        }
    }

    void generateTestNotEqualsDefault(JumpPosition* jumpIfTrue, JumpPosition* jumpIfFalse){
        val1->loadToAccumulator();
        machine.INC();
        val2->subFromAccumulator();
        machine.JZERO(jumpIfTrue); // a < b; pass next condition instructions and jump to instructions
        machine.DEC();
        machine.JZERO(jumpIfFalse);// a = b; jump outside commands
        //a > b; condition satisfied so no jump
    }

    void generateTestEquals(JumpPosition *jumpIfTrue, JumpPosition *jumpIfFalse){
        if(val1->type == Value::Type::NUM || val2->type == Value::Type::NUM){
            this->generateTestEqualsForNumber(jumpIfTrue, jumpIfFalse);
        } else {
            this->generateTestEqualsDefault(jumpIfTrue, jumpIfFalse);
        }
    }

    void generateTestEqualsForNumber(JumpPosition *jumpIfTrue, JumpPosition *jumpIfFalse){
        Value* valNum = val1->type == Value::Type::NUM ? val1 : val2;
        Value* valNotNum = val1->type == Value::Type::NUM ? val2 : val1;

        //optimization for comparing with small numbers
        if(isSmall(valNum)){
            valNotNum->loadToAccumulator();
            //jump to false if already zero
            for(cl_I i = 0; i < valNum->num->num; ++i){
                machine.JZERO(jumpIfFalse);
                machine.DEC();
            }
            machine.JZERO(jumpIfTrue);
            machine.JUMP(jumpIfFalse);
        } else {
            this->generateTestEqualsDefault(jumpIfTrue, jumpIfFalse);
        }
    }

    //atleast lasts 22 time units
    void generateTestEqualsDefault(JumpPosition *jumpIfTrue, JumpPosition *jumpIfFalse){
        val1->loadToAccumulator();
        machine.INC();
        val2->subFromAccumulator();
        machine.JZERO(jumpIfFalse); // a < b; pass next condition instructions and jump after cond
        machine.DEC();
        machine.JZERO(jumpIfTrue); // a = b; pass next condition instructions and jump to instructions
        machine.JUMP(jumpIfFalse); // a > b; pass next condition instructions and jump after cond
    }

    void optimizeConstants(){
        if(val1->type == Value::Type::NUM && val2->type == Value::Type::NUM) {
            this->constComparision = true;
            switch (this->type) {
                case EQ:
                    this->constComparisionResult = val1->num->num == val2->num->num;
                    break;
                case NEQ:
                    this->constComparisionResult = val1->num->num != val2->num->num;
                    break;
                case LT:
                    this->constComparisionResult = val1->num->num < val2->num->num;
                    break;
                case GT:
                    this->constComparisionResult = val1->num->num > val2->num->num;
                    break;
                case LEQ:
                    this->constComparisionResult = val1->num->num <= val2->num->num;
                    break;
                case GEQ:
                    this->constComparisionResult = val1->num->num >= val2->num->num;
                    break;
            }
        }
    }

    bool isComparisionConst(){
        return this->constComparision;
    }

    bool getComparisionConstResult(){
        return this->constComparisionResult;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) {
        this->val1->calculateVariablesUsage(numberOfNestedLoops);
        this->val2->calculateVariablesUsage(numberOfNestedLoops);
    }

    bool propagateConstants(){
        bool hasPropagated = false;
        if(val1->propagateConstant())
            hasPropagated = true;
        if(val2->propagateConstant())
            hasPropagated = true;

        if(hasPropagated)
            cerr << "Constant in " << toString() << " propagated" << endl;

        return hasPropagated;
    }
};
