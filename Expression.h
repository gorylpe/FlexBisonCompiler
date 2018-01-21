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

    Expression(const Expression& expr2)
    :type(expr2.type)
    ,val1(expr2.val1->clone()){
        if(expr2.type != VALUE)
            val2 = expr2.val2->clone();
    }

    Expression* clone(){
        return new Expression(*this);
    }

    string toString(){
        string type;
        switch(this->type){
            case VALUE:
                type = "";
                break;
            case ADDITION:
                type = " + ";
                break;
            case SUBTRACTION:
                type = " - ";
                break;
            case MULTIPLICATION:
                type = " * ";
                break;
            case DIVISION:
                type = " / ";
                break;
            case MODULO:
                type = " % ";
                break;
        }
        string expr;
        switch(this->type){
            case VALUE:
                expr = val1->toString();
                break;
            default:
                expr = val1->toString() + type + val2->toString();
                break;
        }
        return expr;
    }

    bool equals(Expression* expr2){
        if(this->type != expr2->type)
            return false;

        switch(this->type){
            case VALUE:
                if(this->val1->equals(expr2->val1))
                    return true;
            case ADDITION:
            case MULTIPLICATION:
                if(this->val1->equals(expr2->val1) && this->val2->equals(expr2->val2))
                    return true;
                if(this->val2->equals(expr2->val1) && this->val1->equals(expr2->val2))
                    return true;
                break;
            case SUBTRACTION:
            case DIVISION:
            case MODULO:
                if(this->val1->equals(expr2->val1) && this->val2->equals(expr2->val2))
                    return true;
                break;
        }

        return false;
    }

    void semanticAnalysis(){
        switch(type){
            case VALUE:
                val1->semanticAnalysis();
                break;
            default:
                val1->semanticAnalysis();
                val2->semanticAnalysis();
                break;
        }
    }

    void replaceValuesWithConst(string pid, cl_I number){
        switch(type){
            case VALUE:
                val1->replaceIdentifierWithConst(pid, number);
                break;
            default:
                val1->replaceIdentifierWithConst(pid, number);
                val2->replaceIdentifierWithConst(pid, number);
                break;
        }
    }

    void optimizeConstants();

    void bothConstValuesOptimizations();
    void oneConstLeftValueOptimizations();
    void oneConstRightValueOptimizations();

    bool propagateConstants() {
        bool hasPropagated = false;
        switch(this->type){
            case VALUE:
                if(val1->propagateConstant())
                    hasPropagated = true;
                break;
            default:
                if(val1->propagateConstant())
                    hasPropagated = true;
                if(val2->propagateConstant())
                    hasPropagated = true;
                break;
        }
        optimizeConstants();
        if(hasPropagated)
            cerr << "Constant in " << toString() << " propagated" << endl;
        return hasPropagated;
    }

    bool isResultConst(){
        switch(this->type){
            case VALUE:
                return val1->type == Value::Type::NUM;
        }
        return false;
    }

    cl_I getConstValue(){
        return val1->num->num;
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


    void calculateVariablesUsage(cl_I numberOfNestedLoops) {
        switch(this->type){
            case VALUE:
                this->val1->calculateVariablesUsage(numberOfNestedLoops);
                break;
            default:
                this->val1->calculateVariablesUsage(numberOfNestedLoops);
                this->val2->calculateVariablesUsage(numberOfNestedLoops);
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
