#pragma once

#include "Number.h"
#include "Identifier.h"

class Value {
public:
    enum Type{
        NUM = 0,
        IDENTIFIER
    };
    Type type;

    Number* num;
    Identifier* ident;

    explicit Value(Number* num)
    :type(Type::NUM)
    ,num(num){
        cerr << "Creating " << this->toString() << endl;
    }

    explicit Value(Identifier* ident)
    :type(Type::IDENTIFIER)
    ,ident(ident){
        cerr << "Creating " << this->toString() << endl;
    }

    string toString(){
        switch (this->type){
            case NUM:
                return "value - \"" + this->num->toString() + "\"";
            case IDENTIFIER:
                return "value - \"" + this->ident->toString() +"\"";
        }
    }

    void loadToAccumulator(stringstream& ss){
        switch (this->type){
            case Value::Type::NUM:
                this->num->loadToAccumulator(ss);
                break;
            case Value::Type::IDENTIFIER:
                this->ident->loadToAccumulator(ss);
                break;
        }
    }

    cl_I getLoadToAccumulatorLinesCount(){
        switch (this->type){
            case Value::Type::NUM:
                return this->num->getLoadToAccumulatorLinesCount();
            case Value::Type::IDENTIFIER:
                return this->ident->getLoadToAccumulatorLinesCount();
        }
        return 0;
    }

    void addToAccumulator(stringstream& ss){
        switch (this->type){
            case Value::Type::NUM:
                this->num->addToAccumulator(ss);
                break;
            case Value::Type::IDENTIFIER:
                this->ident->addToAccumulator(ss);
                break;
        }
    }

    cl_I getAddToAccumulatorLinesCount(){
        switch (this->type){
            case Value::Type::NUM:
                return this->num->getAddToAccumulatorLinesCount();
            case Value::Type::IDENTIFIER:
                return this->ident->getAddToAccumulatorLinesCount();
        }
        return 0;
    }

    void subFromAccumulator(stringstream& ss){
        switch (this->type){
            case Value::Type::NUM:
                this->num->subFromAccumulator(ss);
                break;
            case Value::Type::IDENTIFIER:
                this->ident->subFromAccumulator(ss);
                break;
        }
    }

    cl_I getSubFromAccumulatorLinesCount(){
        switch (this->type){
            case Value::Type::NUM:
                return this->num->getSubFromAccumulatorLinesCount();
            case Value::Type::IDENTIFIER:
                return this->ident->getSubFromAccumulatorLinesCount();
        }
        return 0;
    }
};

