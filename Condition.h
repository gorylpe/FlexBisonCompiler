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
    ,val2(val2){
        //const comparision optimisation
        if(val1->type == Value::Type::NUM && val2->type == Value::Type::NUM){
            this->constComparisionOptimization();
        }
        cerr << "Creating " << this->toString() << endl;
    }

    string toString(){
        string condType;
        switch (type){
            case EQ:
                condType = "EQUALS";
                break;
            case NEQ:
                condType = "NOT EQUALS";
                break;
            case LT:
                condType = "LESS THAN";
                break;
            case GT:
                condType = "GREATER THAN";
                break;
            case LEQ:
                condType = "LESS THAN OR EQUALS";
                break;
            case GEQ:
                condType = "GREATER THAN OR EQUALS";
                break;
        }
        return "condition " + condType + " of values: " + val1->toString() + ", " + val2->toString();
    }

    void generateTestWithTrueFalseJumps(JumpPosition *jumpIfTrue, JumpPosition *jumpIfFalse) {
        //todo think out eq, make leq and lt same as geq gt
        switch (this->type) {
            case EQ:
                this->generateTestEquals(jumpIfTrue, jumpIfFalse);
                break;
            case NEQ: {
                val1->loadToAccumulator();
                machine.INC();
                val2->subFromAccumulator();
                machine.JZERO(jumpIfTrue); // a < b; pass next condition instructions and jump to instructions
                machine.DEC();
                machine.JZERO(jumpIfFalse);// a = b; jump outside commands
                //a > b; condition satisfied so no jump
            }
                break;
            case LT: {
                val1->loadToAccumulator();
                machine.INC();
                val2->subFromAccumulator();
                machine.JZERO(jumpIfTrue); // a < b
                machine.JUMP(jumpIfFalse); // a >= b
            }
                break;
            case GT: {
                val1->loadToAccumulator();
                val2->subFromAccumulator();
                machine.JZERO(jumpIfFalse);// a <= b; jump outside commands
                // a > b;
            }
                break;
            case LEQ: {
                val1->loadToAccumulator();
                val2->subFromAccumulator();
                machine.JZERO(jumpIfTrue); // a <= b; jump to instructions
                machine.JUMP(jumpIfFalse); // a > b; jump outside instructions
            }
                break;
            case GEQ: {
                val1->loadToAccumulator();
                machine.INC();
                val2->subFromAccumulator();
                machine.JZERO(jumpIfFalse);// a < b;
                // a >= b
            }
                break;
        }
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
        if(valNum->num->num < 2){
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

    void constComparisionOptimization(){
        this->constComparision = true;
        switch(this->type){
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
};
