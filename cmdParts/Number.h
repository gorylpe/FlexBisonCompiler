#pragma once
#include <stack>
#include "../MachineContext.h"
#include "../Utils.h"
#include "../Assembly.h"

using namespace std;
using namespace cln;

class Number {
public:
    Position* pos;
    cl_I num;

    explicit Number(Position* pos, const cl_I& num)
    :pos(pos)
    ,num(num)
    //,isPrepared(false)
    {
        #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Creating number " << this->toString() << endl;
        #endif
    }

    Number(const Number& num2)
    :pos(new Position(*num2.pos))
    ,num(num2.num){}

    Number* clone(){
        return new Number(*this);
    }

    bool equals(Number* num2){
        return this->num == num2->num;
    }

    string toString(){
        stringstream ss;
        ss << this->num;
        return ss.str();
    }

    static stack<int> getBits(cl_I num){
        const cl_I two = 2;

        stack<int> bits;

        while(num > 0){
            cl_I modulo = mod(num, two);
            if(modulo == 1){
                bits.push(1);
            } else {
                bits.push(0);
            }
            num = floor1(num / two);
        }

        return bits;
    }

    static int getLoadToAccumulatorOperations(cl_I num){
        stack<int> bits = getBits(num);

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
        stack<int> bits = getBits(this->num);

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
        return this->getLoadToAccumulatorOperations(this->num) < this->num;
    }

    void addToAccumulator(){
        for(cl_I i = 0; i < this->num; ++i){
            machine.INC();
        }
    }

    //checks if better use DECs or LOAD current accumulator
    bool isLoadStoreInTempBetterThanDecs(){
        return this->getLoadToAccumulatorOperations(this->num) + 10 < this->num; //loadToAcc + STORE
    }

    void subFromAccumulator(){
        for(cl_I i = 0; i < this->num; ++i){
            machine.DEC();
        }
    }
};

