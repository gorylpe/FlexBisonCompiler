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
        cerr << "Creating value number " << toString() << endl;
    }

    explicit Value(Identifier* ident)
    :type(Type::IDENTIFIER)
    ,ident(ident){
        cerr << "Creating value identifier " << toString() << endl;
    }

    string toString(){
        switch (this->type){
            case NUM:
                return this->num->toString();
            case IDENTIFIER:
                return this->ident->toString();
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
        if (this->type == IDENTIFIER){
            this->ident->prepareIfNeeded();
        }
    }

    void unprepareIfNeeded(){
        //free memory after PIDPID type preparation
        if (this->type == IDENTIFIER){
            this->ident->unprepareIfNeeded();
        }
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops){
        if(this->type == IDENTIFIER){
            this->ident->calculateVariablesUsage(numberOfNestedLoops);
        }
    }

    bool propagateConstant(){
        bool hasPropagated = false;

        if(this->type == IDENTIFIER){
            if(this->ident->propagateConstantsInPidpid())
                hasPropagated = true;
            if(this->ident->isConstant()){
                cerr << "Setting constant value " << ident->getConstant() << " to " << toString() << endl;
                this->type = NUM;
                this->num = new Number(ident->pos, ident->getConstant());
                hasPropagated = true;
            }
        }
        return hasPropagated;
    }

    void replaceIdentifierWithConst(string pid, cl_I number){
        if(this->type == IDENTIFIER){
            if(this->ident->pid == pid){

            }
        }
    }

    bool isLoadBetterThanIncs() {
        switch (this->type) {
            case Value::Type::NUM:
                return this->num->isLoadBetterThanIncs();
            case Value::Type::IDENTIFIER:
                return false;
        }
    }

    bool isLoadStoreInTempBetterThanDecs() {
        switch (this->type) {
            case Value::Type::NUM:
                return this->num->isLoadStoreInTempBetterThanDecs();
        }
        return false;
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

