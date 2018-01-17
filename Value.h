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
        cerr << "val " << type << " prep" << endl;
        switch (this->type){
            case Value::Type::NUM:
                //this->num->prepareIfNeeded();
                break;
            case Value::Type::IDENTIFIER:
                this->ident->prepareIfNeeded();
                break;
        }
        cerr << "val " << type << " prep" << endl;
    }

    void unprepareIfNeeded(){
        switch (this->type){
            case Value::Type::NUM:
                //this->num->unprepareIfNeeded();
                break;
            case Value::Type::IDENTIFIER:
                this->ident->unprepareIfNeeded();
                break;
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

