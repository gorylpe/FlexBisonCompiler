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
    void oneConstValueOptimizations();

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

    void loadToAccumulatorValue(stringstream& ss);
    void loadToAccumulatorAddition(stringstream& ss);
    void loadToAccumulatorSubtraction(stringstream& ss);
    void loadToAccumulatorMultiplication(stringstream& ss);
    void loadToAccumulatorDivision(stringstream& ss);
    void loadToAccumulatorModulo(stringstream &ss);

    void loadToAccumulator(stringstream& ss){
        switch (this->type){
            case VALUE:
                this->loadToAccumulatorValue(ss);
                break;
            case ADDITION:
                this->loadToAccumulatorAddition(ss);
                break;
            case SUBTRACTION:
                this->loadToAccumulatorSubtraction(ss);
                break;
            case MULTIPLICATION:
                this->loadToAccumulatorMultiplication(ss);
                break;
            case DIVISION:
                this->loadToAccumulatorDivision(ss);
                break;
            case MODULO:
                this->loadToAccumulatorModulo(ss);
                break;
        }
    }

    cl_I getLoadToAccumulatorValueLinesCount();
    cl_I getLoadToAccumulatorAdditionLinesCount();
    cl_I getLoadToAccumulatorSubtractionLinesCount();
    cl_I getLoadToAccumulatorMultiplicationLinesCount();
    cl_I getLoadToAccumulatorDivisionLinesCount();
    cl_I getLoadToAccumulatorModuloLinesCount();

    cl_I getLoadToAccumulatorLinesCount(){
        cl_I linesCount = 0;
        switch (this->type){
            case VALUE:
                return this->getLoadToAccumulatorValueLinesCount();
            case ADDITION:
                return this->getLoadToAccumulatorAdditionLinesCount();
            case SUBTRACTION:
                return this->getLoadToAccumulatorSubtractionLinesCount();
            case MULTIPLICATION:
                return this->getLoadToAccumulatorMultiplicationLinesCount();
            case DIVISION:
                return this->getLoadToAccumulatorDivisionLinesCount();
            case MODULO:
                return this->getLoadToAccumulatorModuloLinesCount();
        }
        return 0;
    }

};
