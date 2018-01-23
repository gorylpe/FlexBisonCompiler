#pragma once


#include "Value.h"
#include "Assembly.h"
#include "NumberValueStats.h"

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
                break;
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

    bool isTypeVALUE() {
        return type == VALUE;
    }
    bool isTypeADDITION(){
        return type == ADDITION;
    }
    bool isTypeSUBTRACTION(){
        return type == SUBTRACTION;
    }
    bool isTypeMULTIPLICATION(){
        return type == MULTIPLICATION;
    }
    bool isTypeDIVISION(){
        return type == DIVISION;
    }
    bool isTypeMODULO(){
        return type == MODULO;
    }

    bool equals(Identifier* ident2){
        if(this->type == VALUE){
            if(this->val1->type == Value::Type::IDENTIFIER){
                return this->val1->equals(ident2);
            }
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

    void simplifyExpression();

    void bothConstValuesOptimizations();
    void oneConstLeftValueOptimizations();
    void oneConstRightValueOptimizations();
    void addingNumberOptimization();

    vector<Identifier*> getIdentifiers(){
        auto idents = vector<Identifier*>();
        switch(type){
            default: {
                auto ident2 = val2->getIdentifier();
                if (ident2 != nullptr) {
                    idents.push_back(ident2);
                }
            }
            case VALUE:{
                auto ident1 = val1->getIdentifier();
                if(ident1 != nullptr){
                    idents.push_back(ident1);
                }
                break;
            }
        }
        return idents;
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

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) {
        switch(this->type){
            case VALUE:
                if(val1->type == Value::Type::NUM){
                    if(stats.count(val1->num->num) == 0){
                        stats[val1->num->num] = NumberValueStats();
                    }
                    stats[val1->num->num].addLoad(val1, 1);
                }
                break;
            case ADDITION:
                if(val1->type == Value::Type::NUM){
                    if(stats.count(val1->num->num) == 0){
                        stats[val1->num->num] = NumberValueStats();
                    }
                    stats[val1->num->num].addLoad(val1, 1);
                }
                if(val2->type == Value::Type::NUM){
                    if(stats.count(val2->num->num) == 0){
                        stats[val2->num->num] = NumberValueStats();
                    }
                    stats[val2->num->num].addAddition(val2, 1);
                }
                break;
            case SUBTRACTION:
                if(val1->type == Value::Type::NUM){
                    if(stats.count(val1->num->num) == 0){
                        stats[val1->num->num] = NumberValueStats();
                    }
                    stats[val1->num->num].addLoad(val1, 1);
                }
                if(val2->type == Value::Type::NUM){
                    if(stats.count(val2->num->num) == 0){
                        stats[val2->num->num] = NumberValueStats();
                    }
                    stats[val2->num->num].addSubtraction(val2, 1);
                }
                break;
            case DIVISION:
                if(val1->type == Value::Type::NUM){
                    if(stats.count(val1->num->num) == 0){
                        stats[val1->num->num] = NumberValueStats();
                    }
                    stats[val1->num->num].addLoad(val1, 1);
                }
                if(val2->type == Value::Type::NUM){
                    if(!isPowOf2(val2->num->num)) {
                        if (stats.count(val2->num->num) == 0) {
                            stats[val2->num->num] = NumberValueStats();
                        }
                        stats[val2->num->num].addLoad(val2, 1);
                    }
                }
            case MODULO:
                if(val1->type == Value::Type::NUM){
                    if(stats.count(val1->num->num) == 0){
                        stats[val1->num->num] = NumberValueStats();
                    }
                    stats[val1->num->num].addLoad(val1, 1);
                }
                if(val2->type == Value::Type::NUM){
                    if(val2->num->num > 2) {
                        if (stats.count(val2->num->num) == 0) {
                            stats[val2->num->num] = NumberValueStats();
                        }
                        stats[val2->num->num].addSubtraction(val2, 1);
                    }
                }
                break;
        }
    }

    static bool isPowOf2(cl_I num){
        auto numMinus1 = num - 1;
        return (num & numMinus1) == 0;
    }
};
