#pragma once
#include <stack>
#include "MachineContext.h"
#include "Utils.h"
#include "Assembly.h"

using namespace std;
using namespace cln;

class Number {
public:
    //TODO UTWORZENIE TABLICY STALYCH NA POCZATKU PROGRAMU W SPOSOB EFEKTYWNY, POTEM ODWOLYWANIE SIE
    //ALE TYLKO TAKICH KTORYCH UTWORZENIE ZAPISANIE I ODCZYTANIE ZAJMIE DLUZEJ NIZ WIELOKROTNE UTWORZENIE (ZLICZANIE UTWORZEN) 
    //PLUS PROPAGACJA STALYCH
    Position* pos;
    cl_I num;

    explicit Number(Position* pos, const cl_I& num)
    :pos(pos)
    ,num(num){
        cerr << "Creating " << this->toString() << endl;
    }

    bool equals(Number* num2){
        return this->num == num2->num;
    }

    string toString(){
        stringstream ss;
        ss << "num " << this->num;
        return ss.str();
    }

    stack<int> getBits(){
        cl_I tmpnum(this->num);
        const cl_I two = 2;

        stack<int> bits;

        while(tmpnum > 0){
            cl_I modulo = mod(tmpnum, two);
            if(modulo == 1){
                bits.push(1);
            } else {
                bits.push(0);
            }
            tmpnum = floor1(tmpnum / two);
        }

        return bits;
    }

    void loadToAccumulator(){
        stack<int> bits = this->getBits();

        machine.ZERO();

        while(!bits.empty()){
            int bit = bits.top();
            bits.pop();

            if(bit == 1){
                machine.INC();
            }

            if(!bits.empty()){
                machine.SHL();
            }
        }
    }

    void addToAccumulator(){
        if(this->num < 10){
            addToAccumulatorSmall();
        } else {
            addToAccumulatorDefault();
        }
    }

    void addToAccumulatorSmall(){
        for(cl_I i = 0; i < this->num; ++i){
            machine.INC();
        }
    }

    void addToAccumulatorDefault(){
        stack<int> bits = this->getBits();

        Variable* currentAccumulatorValue = memory.pushTempVariable();

        machine.STORE(currentAccumulatorValue->memoryPtr);
        machine.ZERO();

        while(!bits.empty()){
            int bit = bits.top();
            bits.pop();

            if(bit == 1){
                machine.INC();
            }

            if(!bits.empty()){
                machine.SHL();
            }
        }

        machine.ADD(currentAccumulatorValue->memoryPtr);
        memory.popTempVariable(); //currentAccumulatorValue
    }

    void subFromAccumulator(){
        if(this->num < 10){
            subFromAccumulatorSmall();
        } else {
            subFromAccumulatorDefault();
        }
    }

    void subFromAccumulatorSmall(){
        for(cl_I i = 0; i < this->num; ++i){
            machine.DEC();
        }
    }

    void subFromAccumulatorDefault(){
        stack<int> bits = this->getBits();

        Variable* currentAccumulatorValue = memory.pushTempVariable();
        machine.STORE(currentAccumulatorValue->memoryPtr);
        machine.ZERO();

        while(!bits.empty()){
            int bit = bits.top();
            bits.pop();

            if(bit == 1){
                machine.INC();
            }

            if(!bits.empty()){
                machine.SHL();
            }
        }

        Variable* numberValue = memory.pushTempVariable();
        machine.STORE(numberValue->memoryPtr);
        machine.LOAD(currentAccumulatorValue->memoryPtr);
        memory.popTempVariable(); //currentAccumulatorValue
        machine.SUB(numberValue->memoryPtr);
        memory.popTempVariable(); //numberValue
    }
};

