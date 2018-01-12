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
        } else if(val2->type == Value::Type::NUM){
            //val1 is loaded to accumulator when testing conditions
            //so it should be constant if there are comparision with const
            //cause cant add or sub direct value from acc, must be created in there
            Value* swapTmp = this->val2;
            this->val2 = this->val1;
            this->val1 = swapTmp;
            //swap comparision type
            switch(this->type){
                case LT:
                    this->type = GT;
                    break;
                case GT:
                    this->type = LT;
                    break;
                case LEQ:
                    this->type = GEQ;
                    break;
                case GEQ:
                    this->type = LEQ;
                    break;
                default:
                    break;
            }
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

    void generateTestAndJumpIfNotSatisfied(stringstream &ss, const cl_I& jumpLength){
        switch (this->type){
            case EQ: {
                val1->loadToAccumulator(ss);
                INC(ss);
                val2->subFromAccumulator(ss);
                cl_I jumpOutsideCommands = machine.getLineCounter() + 4 + jumpLength;
                JZERO(ss, jumpOutsideCommands); // a < b; pass next condition instructions and jump after cond
                DEC(ss);
                cl_I jumpToCommands = machine.getLineCounter() + 2;
                JZERO(ss, jumpToCommands); // a = b; pass next condition instructions and jump to instructions
                JUMP(ss, jumpOutsideCommands); // a > b; pass next condition instructions and jump after cond
            }
                break;
            case NEQ: {
                val1->loadToAccumulator(ss);
                INC(ss);
                val2->subFromAccumulator(ss);
                cl_I jumpToCommands = machine.getLineCounter() + 3;
                JZERO(ss, jumpToCommands); // a < b; pass next condition instructions and jump to instructions
                DEC(ss);
                cl_I jumpOutsideCommands = machine.getLineCounter() + 1 + jumpLength;
                JZERO(ss, jumpOutsideCommands);// a = b; jump outside commands
                //a > b; condition satisfied so no jump
            }
                break;
            case LT: {
                val1->loadToAccumulator(ss);
                INC(ss);
                val2->subFromAccumulator(ss);
                cl_I jumpToCommands = machine.getLineCounter() + 2;
                JZERO(ss, jumpToCommands); // a < b
                cl_I jumpOutsideCommands = machine.getLineCounter() + 1 + jumpLength;
                JUMP(ss, jumpOutsideCommands); // a >= b
            }
                break;
            case GT: {
                val1->loadToAccumulator(ss);
                val2->subFromAccumulator(ss);
                cl_I jumpOutsideCommands = machine.getLineCounter() + 1 + jumpLength;
                JZERO(ss, jumpOutsideCommands);// a <= b; jump outside commands
                // a > b;
            }
                break;
            case LEQ: {
                val1->loadToAccumulator(ss);
                val2->subFromAccumulator(ss);
                cl_I jumpToCommands = machine.getLineCounter() + 2;
                JZERO(ss, jumpToCommands); // a <= b; jump to instructions
                cl_I jumpOutsideCommands = machine.getLineCounter() + 1 + jumpLength;
                JUMP(ss, jumpOutsideCommands); // a > b; jump outside instructions
            }
                break;
            case GEQ: {
                val1->loadToAccumulator(ss);
                INC(ss);
                val2->subFromAccumulator(ss);
                cl_I jumpOutsideCommands = machine.getLineCounter() + 1 + jumpLength;
                JZERO(ss, jumpOutsideCommands);// a < b;
                // a >= b
            }
                break;
        }
    }

    cl_I getTestAndJumpIfNotSatisfiedLinesCount(){
        cl_I linesCount = 0;
        switch (this->type) {
            case EQ:
                linesCount += val1->getLoadToAccumulatorLinesCount();
                linesCount += 1; //INC
                linesCount += val2->getSubFromAccumulatorLinesCount();
                linesCount += 4; //JZERO DEC JZERO JUMP
                break;
            case NEQ:
                linesCount += val1->getLoadToAccumulatorLinesCount();
                linesCount += 1; //INC
                linesCount += val2->getSubFromAccumulatorLinesCount();
                linesCount += 3; //JZERO DEC JZERO
                break;
            case LT:
                linesCount += val1->getLoadToAccumulatorLinesCount();
                linesCount += 1; //INC
                linesCount += val2->getSubFromAccumulatorLinesCount();
                linesCount += 2; //JZERO JUMP
                break;
            case GT:
                linesCount += val1->getLoadToAccumulatorLinesCount();
                linesCount += val2->getSubFromAccumulatorLinesCount();
                linesCount += 1; //JZERO
                break;
            case LEQ:
                linesCount += val1->getLoadToAccumulatorLinesCount();
                linesCount += val2->getSubFromAccumulatorLinesCount();
                linesCount += 2; //JZERO JUMP
                break;
            case GEQ:
                linesCount += val1->getLoadToAccumulatorLinesCount();
                linesCount += 1; //INC
                linesCount += val2->getSubFromAccumulatorLinesCount();
                linesCount += 1; //JZERO
                break;
        }
        return linesCount;
    }
};
