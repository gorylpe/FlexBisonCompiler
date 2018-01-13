//
// Created by piotr on 17.12.2017.
//

#include "Expression.h"

Expression::Expression(Value *val)
        :type(VALUE)
        ,val1(val){
    cerr << "Creating " << this->toString() << endl;
}

Expression::Expression(Expression::Type type, Value *val1, Value *val2)
        :type(type)
        ,val1(val1)
        ,val2(val2){
    //const operation optimisation
    if(val1->type == Value::Type::NUM && val2->type == Value::Type::NUM) {

        this->bothConstValuesOptimizations();
    } else if (val2->type == Value::Type::NUM || val1->type == Value::Type::NUM){

        this->oneConstValueOptimizations();
    }

    cerr << "Creating " << this->toString() << endl;
}


void Expression::bothConstValuesOptimizations() {
    switch(this->type){
        case ADDITION:
            val1->num->num += val2->num->num;
            break;
        case SUBTRACTION:
            val1->num->num = max(val1->num->num - val2->num->num, 0);
            break;
        case MULTIPLICATION:
            val1->num->num *= val2->num->num;
            break;
        case DIVISION:
            if(val2->num->num == 0){
                val1->num->num = 0;
            } else {
                val1->num->num = floor1(val1->num->num / val2->num->num);
            }
            break;
        case MODULO:
            if(val2->num->num == 0){
                val1->num->num = 0;
            } else {
                val1->num->num -= floor1(val1->num->num / val2->num->num) * val2->num->num;
            }
            break;
        default:
            break;
    }
    //leave single val1
    this->type = VALUE;
}

void Expression::oneConstValueOptimizations(){
    bool swapped = false;
    if(this->val1->type == Value::Type::NUM){
        //swap for more general code
        Value* tmp = this->val1;
        this->val1 = this->val2;
        this->val2 = tmp;
        swapped = true;
    }
    //some operations optimisations
    switch(this->type){
        case ADDITION:
        case SUBTRACTION:
            if(this->val2->num->num == 0){
                //leave single val1
                this->type = VALUE;
            }
            break;
        case MULTIPLICATION:
        case DIVISION:
        case MODULO:
            if(this->val2->num->num == 0){
                //leave single val1
                this->type = VALUE;
                if(this->val1->type == Value::Type::NUM){
                    this->val1->num->num = 0;
                } else {
                    this->val1 = new Value(new Number(this->val1->ident->pos, 0));
                }
            } else if(this->val2->num->num == 1){
                //leave single val1
                this->type = VALUE;
            }
            break;
        default:
            break;
    }
    //unswap
    if(swapped){
        Value* tmp = this->val1;
        this->val1 = this->val2;
        this->val2 = tmp;
    }
}

void Expression::loadToAccumulatorValue() {
    this->val1->loadToAccumulator();
}

void Expression::loadToAccumulatorAddition() {
    this->val1->loadToAccumulator();
    this->val2->addToAccumulator();
}

void Expression::loadToAccumulatorSubtraction() {
    this->val1->loadToAccumulator();
    this->val2->subFromAccumulator();
}

void Expression::loadToAccumulatorMultiplication() {
    //TODO optimization for multiplication by power of 2
    //optimization for number
    auto tmpVal1 = memory.pushTempVariable();
    auto tmpVal2 = memory.pushTempVariable();

    this->val1->loadToAccumulator();
    machine.STORE(tmpVal1->memoryPtr);
    this->val2->loadToAccumulator();
    machine.STORE(tmpVal2->memoryPtr);

    auto currentShiftedNumber = memory.pushTempVariable();
    auto bits = memory.pushTempVariable();
    auto result = memory.pushTempVariable();

    auto value1LessOrEqualsValue2 = new JumpPosition();
    auto multiplicationStart = new JumpPosition();
    auto multiplicationLoopStart = new JumpPosition();
    auto multiplicationEnd = new JumpPosition();
    auto multiplicationAdding = new JumpPosition();
    auto multiplicationShifting = new JumpPosition();


    //compare values to chose which one is smaller for faster logarithmic multiplication
    machine.LOAD(tmpVal1->memoryPtr);
    machine.SUB(tmpVal2->memoryPtr);
    //val1 <= val2
    machine.JZERO(value1LessOrEqualsValue2);

    //val1 > val2
    machine.LOAD(tmpVal1->memoryPtr);
    machine.STORE(currentShiftedNumber->memoryPtr);
    machine.LOAD(tmpVal2->memoryPtr);
    machine.STORE(bits->memoryPtr);
    machine.JUMP(multiplicationStart);

    machine.setJumpPosition(value1LessOrEqualsValue2);
    //val1 <= val2
    machine.LOAD(tmpVal1->memoryPtr);
    machine.STORE(bits->memoryPtr);
    machine.LOAD(tmpVal2->memoryPtr);
    machine.STORE(currentShiftedNumber->memoryPtr);


    machine.setJumpPosition(multiplicationStart);
    //multiplication, init result
    machine.ZERO();
    machine.STORE(result->memoryPtr);

    machine.LOAD(bits->memoryPtr);

    //bits from last loop are loaded in accumulator so doesnt need to load again
    machine.setJumpPosition(multiplicationLoopStart);

    //when multiplier with bits is 0 jump to end
    machine.JZERO(multiplicationEnd);

    //last bit = 1, jump to adding and shifting
    machine.JODD(multiplicationAdding);

    //last bit = 0, jump to only shifting
    machine.JUMP(multiplicationShifting);

    //adding
    machine.setJumpPosition(multiplicationAdding);
    machine.LOAD(result->memoryPtr);
    machine.ADD(currentShiftedNumber->memoryPtr);
    machine.STORE(result->memoryPtr);

    //shifting number
    machine.setJumpPosition(multiplicationShifting);
    machine.LOAD(currentShiftedNumber->memoryPtr);
    machine.SHL();
    machine.STORE(currentShiftedNumber->memoryPtr);
    //shifting bits
    machine.LOAD(bits->memoryPtr);
    machine.SHR();
    machine.STORE(bits->memoryPtr);

    machine.JUMP(multiplicationLoopStart);

    machine.setJumpPosition(multiplicationEnd);
    machine.LOAD(result->memoryPtr);

    memory.popTempVariable(); //tmpVal1
    memory.popTempVariable(); //tmpVal2
    memory.popTempVariable(); //currentShiftedNumber
    memory.popTempVariable(); //bits
    memory.popTempVariable(); //result
}

void Expression::loadToAccumulatorDivision() {
    //TODO optimisations pow 2
    auto currentBit = memory.pushTempVariable();
    auto currentDividend = memory.pushTempVariable();
    auto currentDivider = memory.pushTempVariable();
    auto result = memory.pushTempVariable();

    auto firstDividerDividendComparision = new JumpPosition();
    auto dividerShiftingStart = new JumpPosition();
    auto divisionShiftingStart = new JumpPosition();
    auto divisionSubtraction = new JumpPosition();
    auto divisionEnd = new JumpPosition();

    this->val1->loadToAccumulator();
    machine.STORE(currentDividend->memoryPtr);

    this->val2->loadToAccumulator();
    machine.STORE(currentDivider->memoryPtr);

    machine.ZERO();
    machine.STORE(result->memoryPtr);
    machine.INC();
    machine.STORE(currentBit->memoryPtr);

    machine.JUMP(firstDividerDividendComparision);

    //shift current bit and divider, and to divider must be <= divident
    machine.setJumpPosition(dividerShiftingStart);
    machine.LOAD(currentBit->memoryPtr);
    machine.SHL();
    machine.STORE(currentBit->memoryPtr);
    machine.LOAD(currentDivider->memoryPtr);
    machine.SHL();
    machine.STORE(currentDivider->memoryPtr);

    //jumping here first time
    machine.setJumpPosition(firstDividerDividendComparision);
    machine.LOAD(currentDivider->memoryPtr);
    machine.SUB(currentDividend->memoryPtr);
    //divider <= dividend
    machine.JZERO(dividerShiftingStart);

    //divider > dividend, so shift once right
    machine.setJumpPosition(divisionShiftingStart);
    machine.LOAD(currentBit->memoryPtr);
    machine.SHR();
    machine.STORE(currentBit->memoryPtr);
    machine.LOAD(currentDivider->memoryPtr);
    machine.SHR();
    machine.STORE(currentDivider->memoryPtr);

    machine.LOAD(currentBit->memoryPtr);
    //while currentBit > 0
    //jump if division ended
    machine.JZERO(divisionEnd);
    //check if divider > dividend, jump to shift right, else subtract and add current bit to result
    machine.LOAD(currentDivider->memoryPtr);
    machine.SUB(currentDividend->memoryPtr);
    //divider <= dividend, jump to subtract
    machine.JZERO(divisionSubtraction);
    //divider > dividend
    machine.JUMP(divisionShiftingStart);
    //subtraction
    machine.setJumpPosition(divisionSubtraction);
    machine.LOAD(currentDividend->memoryPtr);
    machine.SUB(currentDivider->memoryPtr);
    machine.STORE(currentDividend->memoryPtr);
    //add bit to result
    machine.LOAD(result->memoryPtr);
    machine.ADD(currentBit->memoryPtr);
    machine.STORE(result->memoryPtr);
    machine.JUMP(divisionShiftingStart);

    machine.setJumpPosition(divisionEnd);
    machine.LOAD(result->memoryPtr);

    memory.popTempVariable(); //currentBit
    memory.popTempVariable(); //currentDividend
    memory.popTempVariable(); //currentDivider
    memory.popTempVariable(); //result
}

void Expression::loadToAccumulatorModulo() {
    //TODO optimisations pow 2
    Variable* currentBit = memory.pushTempVariable();
    Variable* currentDividend = memory.pushTempVariable();
    Variable* currentDivider = memory.pushTempVariable();
    Variable* result = memory.pushTempVariable();

    auto firstDividerDividendComparision = new JumpPosition();
    auto dividerShiftingStart = new JumpPosition();
    auto divisionShiftingStart = new JumpPosition();
    auto divisionSubtraction = new JumpPosition();
    auto divisionEnd = new JumpPosition();

    this->val1->loadToAccumulator();
    machine.STORE(currentDividend->memoryPtr);

    this->val2->loadToAccumulator();
    machine.STORE(currentDivider->memoryPtr);

    machine.ZERO();
    machine.STORE(result->memoryPtr);
    machine.INC();
    machine.STORE(currentBit->memoryPtr);

    machine.JUMP(firstDividerDividendComparision);

    //shift current bit and divider, and to divider must be <= divident
    machine.setJumpPosition(dividerShiftingStart);
    machine.LOAD(currentBit->memoryPtr);
    machine.SHL();
    machine.STORE(currentBit->memoryPtr);
    machine.LOAD(currentDivider->memoryPtr);
    machine.SHL();
    machine.STORE(currentDivider->memoryPtr);

    //jumping here first time
    machine.setJumpPosition(firstDividerDividendComparision);
    machine.LOAD(currentDivider->memoryPtr);
    machine.SUB(currentDividend->memoryPtr);
    //divider <= dividend
    machine.JZERO(dividerShiftingStart);

    //divider > dividend, so shift once right
    machine.setJumpPosition(divisionShiftingStart);
    machine.LOAD(currentBit->memoryPtr);
    machine.SHR();
    machine.STORE(currentBit->memoryPtr);
    machine.LOAD(currentDivider->memoryPtr);
    machine.SHR();
    machine.STORE(currentDivider->memoryPtr);

    machine.LOAD(currentBit->memoryPtr);
    //while currentBit > 0
    //jump if division ended
    machine.JZERO(divisionEnd);
    //check if divider > dividend, jump to shift right, else subtract and add current bit to result
    machine.LOAD(currentDivider->memoryPtr);
    machine.SUB(currentDividend->memoryPtr);
    //divider <= dividend, jump to subtract
    machine.JZERO(divisionSubtraction);
    //divider > dividend
    machine.JUMP(divisionShiftingStart);
    //subtraction
    machine.setJumpPosition(divisionSubtraction);
    machine.LOAD(currentDividend->memoryPtr);
    machine.SUB(currentDivider->memoryPtr);
    machine.STORE(currentDividend->memoryPtr);
    //jump to shifting start
    machine.JUMP(divisionShiftingStart);

    machine.setJumpPosition(divisionEnd);
    machine.LOAD(currentDividend->memoryPtr);

    memory.popTempVariable(); //currentBit
    memory.popTempVariable(); //currentDividend
    memory.popTempVariable(); //currentDivider
    memory.popTempVariable(); //result
}


