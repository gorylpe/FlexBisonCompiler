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
    ,num(num)
    //,isPrepared(false)
    {
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

    int getLoadToAccumulatorOperations(){
        stack<int> bits = this->getBits();

        int numOfOps = 0;
        numOfOps++; //ZERO

        while(!bits.empty()){
            int bit = bits.top();
            bits.pop();

            if(bit == 1){
                numOfOps++; //INC
            }

            if(!bits.empty()){
                numOfOps++; //SHL
            }
        }

        return numOfOps;
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

    //checks if better use INCs or LOAD current accumulator
    bool isLoadBetterThanIncs(){
        return this->getLoadToAccumulatorOperations() < this->num;
    }

    void addToAccumulator(){
        for(cl_I i = 0; i < this->num; ++i){
            machine.INC();
        }
    }

    //checks if better use DECs or LOAD current accumulator
    bool isLoadStoreInTempBetterThanDecs(){
        return this->getLoadToAccumulatorOperations() + 10 < this->num; //loadToAcc + STORE
    }

    void subFromAccumulator(){
        for(cl_I i = 0; i < this->num; ++i){
            machine.DEC();
        }
    }
};

