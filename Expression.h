#pragma once


#include "Value.h"
#include "Assembly.h"

class Expression {
public:
    enum Type{
        VALUE = 0,
        ADDITION,
        SUBTRACTION,
        MULTIPLICATION,
        DIVISION,
        MODULO
    };

    Type type;
    Value* val1;
    Value* val2;

    explicit Expression(Value* val);

    explicit Expression(Type type, Value* val1, Value* val2);

    void bothConstValuesOptimizations();
    void oneConstLeftValueOptimizations();
    void oneConstRightValueOptimizations();

    string toString(){
        string values;
        switch(this->type){
            case VALUE:
                values = val1->toString();
                break;
            default:
                values = val1->toString() + ", " + val2->toString();
                break;
        }
        string type;
        switch(this->type){
            case VALUE:
                type = "VALUE";
                break;
            case ADDITION:
                type = "ADDITION";
                break;
            case SUBTRACTION:
                type = "SUBTRACTION";
                break;
            case MULTIPLICATION:
                type = "MULTIPLICATION";
                break;
            case DIVISION:
                type = "DIVISION";
                break;
            case MODULO:
                type = "MODULO";
                break;
        }
        return "expression " + type + " with " + values;
    }

    void prepareValuesIfNeeded(){
        switch(this->type){
            case VALUE:
                this->val1->prepareIfNeeded();
                break;
            default:
                this->val1->prepareIfNeeded();
                this->val2->prepareIfNeeded();
                break;
        }
    }

    void unprepareValuesIfNeeded(){
        switch(this->type){
            case VALUE:
                this->val1->unprepareIfNeeded();
                break;
            default:
                this->val1->unprepareIfNeeded();
                this->val2->unprepareIfNeeded();
                break;
        }
    }

    void loadToAccumulatorValue();
    void loadToAccumulatorAddition();
    void loadToAccumulatorSubtraction();
    void loadToAccumulatorMultiplication();
    void loadToAccumulatorDivision();
    void loadToAccumulatorModulo();

    void loadToAccumulator(){
        switch (this->type){
            case VALUE:
                this->loadToAccumulatorValue();
                break;
            case ADDITION:
                this->loadToAccumulatorAddition();
                break;
            case SUBTRACTION:
                this->loadToAccumulatorSubtraction();
                break;
            case MULTIPLICATION:
                this->loadToAccumulatorMultiplication();
                break;
            case DIVISION:
                this->loadToAccumulatorDivision();
                break;
            case MODULO:
                this->loadToAccumulatorModulo();
                break;
        }
    }
};
