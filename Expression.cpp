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

void Expression::loadToAccumulatorValue(stringstream &ss) {
    this->val1->loadToAccumulator(ss);
}

cl_I Expression::getLoadToAccumulatorValueLinesCount() {
    return this->val1->getLoadToAccumulatorLinesCount();
}

void Expression::loadToAccumulatorAddition(stringstream &ss) {
    this->val1->loadToAccumulator(ss);
    this->val2->addToAccumulator(ss);
}

cl_I Expression::getLoadToAccumulatorAdditionLinesCount() {
    cl_I linesCount = 0;
    linesCount += this->val1->getLoadToAccumulatorLinesCount();
    linesCount += this->val2->getAddToAccumulatorLinesCount();
    return linesCount;
}

void Expression::loadToAccumulatorSubtraction(stringstream &ss) {
    this->val1->loadToAccumulator(ss);
    this->val2->subFromAccumulator(ss);
}

cl_I Expression::getLoadToAccumulatorSubtractionLinesCount() {
    cl_I linesCount = 0;
    linesCount += this->val1->getLoadToAccumulatorLinesCount();
    linesCount += this->val2->getSubFromAccumulatorLinesCount();
    return linesCount;
}

void Expression::loadToAccumulatorMultiplication(stringstream& ss) {
    //TODO optimization for multiplication by power of 2
    //optimization for number
    Variable* tmpVal1 = memory.pushTempVariable();
    Variable* tmpVal2 = memory.pushTempVariable();

    this->val1->loadToAccumulator(ss);
    STORE(ss, tmpVal1->memoryPtr);
    this->val2->loadToAccumulator(ss);
    STORE(ss, tmpVal2->memoryPtr);

    Variable* currentShiftedNumber = memory.pushTempVariable();
    Variable* bits = memory.pushTempVariable();
    Variable* result = memory.pushTempVariable();


    //compare values to chose which one is smaller for faster logarithmic multiplication
    LOAD(ss, tmpVal1->memoryPtr);
    SUB(ss, tmpVal2->memoryPtr);
    //val1 <= val2
    cl_I jumpToLessOrEquals = machine.getLineCounter() + 6;
    JZERO(ss, jumpToLessOrEquals);

    //val1 > val2
    LOAD(ss, tmpVal1->memoryPtr);
    STORE(ss, currentShiftedNumber->memoryPtr);
    LOAD(ss, tmpVal2->memoryPtr);
    STORE(ss, bits->memoryPtr);

    cl_I jumpToMultiplication = machine.getLineCounter() + 5;
    JUMP(ss, jumpToMultiplication);

    //val1 <= val2
    LOAD(ss, tmpVal1->memoryPtr);
    STORE(ss, bits->memoryPtr);
    LOAD(ss, tmpVal2->memoryPtr);
    STORE(ss, currentShiftedNumber->memoryPtr);


    //multiplication, init result
    ZERO(ss);
    STORE(ss, result->memoryPtr);

    LOAD(ss, bits->memoryPtr);

    //bits from last loop are loaded in accumulator so doesnt need to load again
    cl_I multiplicationStart = machine.getLineCounter();

    //when multiplier with bits is 0 jump to end
    cl_I jumpToMultiplicationEnd = machine.getLineCounter() + 13;
    JZERO(ss, jumpToMultiplicationEnd);

    //last bit = 1, jump to adding and shifting
    cl_I jumpToAdding = machine.getLineCounter() + 2;
    JODD(ss, jumpToAdding);

    //last bit = 0, jump to only shifting
    cl_I jumpToShifting = machine.getLineCounter() + 4;
    JUMP(ss, jumpToShifting);

    //adding
    LOAD(ss, result->memoryPtr);
    ADD(ss, currentShiftedNumber->memoryPtr);
    STORE(ss, result->memoryPtr);

    //shifting number
    LOAD(ss, currentShiftedNumber->memoryPtr);
    SHL(ss);
    STORE(ss, currentShiftedNumber->memoryPtr);
    //shifting bits
    LOAD(ss, bits->memoryPtr);
    SHR(ss);
    STORE(ss, bits->memoryPtr);

    //jump to start of loop
    JUMP(ss, multiplicationStart);

    //result to accumulator
    LOAD(ss, result->memoryPtr);

    memory.popTempVariable(); //tmpVal1
    memory.popTempVariable(); //tmpVal2
    memory.popTempVariable(); //currentShiftedNumber
    memory.popTempVariable(); //bits
    memory.popTempVariable(); //result
}

cl_I Expression::getLoadToAccumulatorMultiplicationLinesCount() {
    cl_I linesCount = 0;
    linesCount += this->val1->getLoadToAccumulatorLinesCount();
    linesCount += this->val2->getLoadToAccumulatorLinesCount();
    linesCount += 31;
    return linesCount;
}

void Expression::loadToAccumulatorDivision(stringstream &ss) {
    //TODO optimisations pow 2
    Variable* currentBit = memory.pushTempVariable();
    Variable* currentDividend = memory.pushTempVariable();
    Variable* currentDivider = memory.pushTempVariable();
    Variable* result = memory.pushTempVariable();

    this->val1->loadToAccumulator(ss);
    STORE(ss, currentDividend->memoryPtr);

    this->val2->loadToAccumulator(ss);
    STORE(ss, currentDivider->memoryPtr);

    ZERO(ss);
    STORE(ss, result->memoryPtr);
    INC(ss);
    STORE(ss, currentBit->memoryPtr);

    cl_I firstDividerDividendComparision = machine.getLineCounter() + 7;
    JUMP(ss, firstDividerDividendComparision);

    //shift current bit and divider, and to divider must be <= divident
    cl_I dividerShiftingStart = machine.getLineCounter();

    //shifting divider and bit
    LOAD(ss, currentBit->memoryPtr);
    SHL(ss);
    STORE(ss, currentBit->memoryPtr);
    LOAD(ss, currentDivider->memoryPtr);
    SHL(ss);
    STORE(ss, currentDivider->memoryPtr);

    //jumping here first time
    LOAD(ss, currentDivider->memoryPtr);
    SUB(ss, currentDividend->memoryPtr);
    //divider <= dividend
    JZERO(ss, dividerShiftingStart);

    cl_I divisionShiftingStart = machine.getLineCounter();
    //divider > dividend, so shift once right
    LOAD(ss, currentBit->memoryPtr);
    SHR(ss);
    STORE(ss, currentBit->memoryPtr);
    LOAD(ss, currentDivider->memoryPtr);
    SHR(ss);
    STORE(ss, currentDivider->memoryPtr);


    //while currentBit > 0
    cl_I outsideDivision = machine.getLineCounter() + 13;
    LOAD(ss, currentBit->memoryPtr);
    //jump if division ended
    JZERO(ss, outsideDivision);
    //check if divider > dividend, jump to shift right, else subtract and add current bit to result
    LOAD(ss, currentDivider->memoryPtr);
    SUB(ss, currentDividend->memoryPtr);
    //divider <= dividend, jump to subtract
    cl_I subtracting = machine.getLineCounter() + 2;
    JZERO(ss, subtracting);
    //divider > dividend
    JUMP(ss, divisionShiftingStart);
    //subtraction
    LOAD(ss, currentDividend->memoryPtr);
    SUB(ss, currentDivider->memoryPtr);
    STORE(ss, currentDividend->memoryPtr);
    //add bit to result
    LOAD(ss, result->memoryPtr);
    ADD(ss, currentBit->memoryPtr);
    STORE(ss, result->memoryPtr);
    JUMP(ss, divisionShiftingStart);

    LOAD(ss, result->memoryPtr);

    memory.popTempVariable(); //currentBit
    memory.popTempVariable(); //currentDividend
    memory.popTempVariable(); //currentDivider
    memory.popTempVariable(); //result
}

cl_I Expression::getLoadToAccumulatorDivisionLinesCount() {
    cl_I linesCount = 0;
    linesCount += this->val1->getLoadToAccumulatorLinesCount();
    linesCount += this->val2->getLoadToAccumulatorLinesCount();
    linesCount += 36;
    return linesCount;
}



void Expression::loadToAccumulatorModulo(stringstream &ss) {
    //TODO optimisations pow 2
    Variable* currentBit = memory.pushTempVariable();
    Variable* currentDividend = memory.pushTempVariable();
    Variable* currentDivider = memory.pushTempVariable();
    Variable* result = memory.pushTempVariable();

    this->val1->loadToAccumulator(ss);
    STORE(ss, currentDividend->memoryPtr);

    this->val2->loadToAccumulator(ss);
    STORE(ss, currentDivider->memoryPtr);

    ZERO(ss);
    STORE(ss, result->memoryPtr);
    INC(ss);
    STORE(ss, currentBit->memoryPtr);

    cl_I firstDividerDividendComparision = machine.getLineCounter() + 7;
    JUMP(ss, firstDividerDividendComparision);

    //shift current bit and divider, and to divider must be <= divident
    cl_I dividerShiftingStart = machine.getLineCounter();

    //shifting divider and bit
    LOAD(ss, currentBit->memoryPtr);
    SHL(ss);
    STORE(ss, currentBit->memoryPtr);
    LOAD(ss, currentDivider->memoryPtr);
    SHL(ss);
    STORE(ss, currentDivider->memoryPtr);

    //jumping here first time
    LOAD(ss, currentDivider->memoryPtr);
    SUB(ss, currentDividend->memoryPtr);
    //divider <= dividend
    JZERO(ss, dividerShiftingStart);

    cl_I divisionShiftingStart = machine.getLineCounter();
    //divider > dividend, so shift once right
    LOAD(ss, currentBit->memoryPtr);
    SHR(ss);
    STORE(ss, currentBit->memoryPtr);
    LOAD(ss, currentDivider->memoryPtr);
    SHR(ss);
    STORE(ss, currentDivider->memoryPtr);


    //while currentBit > 0
    cl_I outsideDivision = machine.getLineCounter() + 10;
    LOAD(ss, currentBit->memoryPtr);
    //jump if division ended
    JZERO(ss, outsideDivision);
    //check if divider > dividend, jump to shift right, else subtract and add current bit to result
    LOAD(ss, currentDivider->memoryPtr);
    SUB(ss, currentDividend->memoryPtr);
    //divider <= dividend, jump to subtract
    cl_I subtracting = machine.getLineCounter() + 2;
    JZERO(ss, subtracting);
    //divider > dividend
    JUMP(ss, divisionShiftingStart);
    //subtraction
    LOAD(ss, currentDividend->memoryPtr);
    SUB(ss, currentDivider->memoryPtr);
    STORE(ss, currentDividend->memoryPtr);
    //jump to shifting start
    JUMP(ss, divisionShiftingStart);

    //load
    LOAD(ss, currentDividend->memoryPtr);

    memory.popTempVariable(); //currentBit
    memory.popTempVariable(); //currentDividend
    memory.popTempVariable(); //currentDivider
    memory.popTempVariable(); //result
}

cl_I Expression::getLoadToAccumulatorModuloLinesCount() {
    cl_I linesCount = 0;
    linesCount += this->val1->getLoadToAccumulatorLinesCount();
    linesCount += this->val2->getLoadToAccumulatorLinesCount();
    linesCount += 33;
    return linesCount;
}


