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

    string toString(){
        stringstream ss;
        ss << "num " << this->num;
        return ss.str();
    }

    void loadToAccumulator(stringstream& ss){
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

        ZERO(ss);

        while(!bits.empty()){
            int bit = bits.top();
            bits.pop();

            if(bit == 1){
                INC(ss);
            }

            if(!bits.empty()){
                SHL(ss);
            }
        }
    }

    void addToAccumulator(stringstream& ss){
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

        Variable* currentAccumulatorValue = memory.pushTempVariable();

        STORE(ss, currentAccumulatorValue->memoryPtr);
        ZERO(ss);

        while(!bits.empty()){
            int bit = bits.top();
            bits.pop();

            if(bit == 1){
                INC(ss);
            }

            if(!bits.empty()){
                SHL(ss);
            }
        }

        ADD(ss, currentAccumulatorValue->memoryPtr);
        memory.popTempVariable(); //currentAccumulatorValue
    }

    void subFromAccumulator(stringstream& ss){
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

        Variable* currentAccumulatorValue = memory.pushTempVariable();
        STORE(ss, currentAccumulatorValue->memoryPtr);
        ZERO(ss);

        while(!bits.empty()){
            int bit = bits.top();
            bits.pop();

            if(bit == 1){
                INC(ss);
            }

            if(!bits.empty()){
                SHL(ss);
            }
        }

        Variable* numberValue = memory.pushTempVariable();
        STORE(ss, numberValue->memoryPtr);
        LOAD(ss, currentAccumulatorValue->memoryPtr);
        memory.popTempVariable(); //currentAccumulatorValue
        SUB(ss, numberValue->memoryPtr);
        memory.popTempVariable(); //numberValue
    }

    cl_I getLoadToAccumulatorLinesCount(){
        cl_I count = 0;
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

        count++;

        while(!bits.empty()){
            int bit = bits.top();
            bits.pop();

            if(bit == 1){
                count++;
            }

            if(!bits.empty()){
                count++;
            }
        }

        return count;
    }

    cl_I getAddToAccumulatorLinesCount(){
        cl_I count = 0;
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

        count++;
        count++;

        while(!bits.empty()){
            int bit = bits.top();
            bits.pop();

            if(bit == 1){
                count++;
            }

            if(!bits.empty()){
                count++;
            }
        }

        count++;

        return count;
    }

    cl_I getSubFromAccumulatorLinesCount(){
        cl_I count = 0;
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

        count++;
        count++;

        while(!bits.empty()){
            int bit = bits.top();
            bits.pop();

            if(bit == 1){
                count++;
            }

            if(!bits.empty()){
                count++;
            }
        }

        count++;
        count++;
        count++;

        return count;
    }
};

