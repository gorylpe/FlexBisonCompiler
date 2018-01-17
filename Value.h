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

    bool equals(Value* val2){
        if(this->type != val2->type){
            return false;
        } else {
            if(this->type == NUM){
                return this->num->equals(val2->num);
            } else if(this->type == IDENTIFIER){
                return this->ident->equals(val2->ident);
            }
        }
    }

    void prepareIfNeeded(){
        //prepare for PIDPID type
        if (this->type == Value::Type::IDENTIFIER){
            this->ident->prepareIfNeeded();
        }
    }

    void unprepareIfNeeded(){
        //free memory after PIDPID type preparation
        if (this->type == Value::Type::IDENTIFIER){
            this->ident->unprepareIfNeeded();
        }
    }

    bool isLoadBetterThanAdd() {
        switch (this->type) {
            case Value::Type::NUM:
                return this->num->isLoadBetterThanAdd();
            case Value::Type::IDENTIFIER:
                return this->ident->isLoadBetterThanAdd();
        }
    }

    void loadToAccumulator(){
        switch (this->type){
            case Value::Type::NUM:
                this->num->loadToAccumulator();
                break;
            case Value::Type::IDENTIFIER:
                this->ident->loadToAccumulator();
                break;
        }
    }

    void addToAccumulator(){
        switch (this->type){
            case Value::Type::NUM:
                this->num->addToAccumulator();
                break;
            case Value::Type::IDENTIFIER:
                this->ident->addToAccumulator();
                break;
        }
    }

    void subFromAccumulator(){
        switch (this->type){
            case Value::Type::NUM:
                this->num->subFromAccumulator();
                break;
            case Value::Type::IDENTIFIER:
                this->ident->subFromAccumulator();
                break;
        }
    }
};

