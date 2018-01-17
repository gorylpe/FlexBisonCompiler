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
    } else if (val1->type == Value::Type::NUM) {

        this->oneConstLeftValueOptimizations();
    } else if (val2->type == Value::Type::NUM) {

        this->oneConstRightValueOptimizations();
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

void Expression::oneConstLeftValueOptimizations(){
    //some operations optimisations
    switch(this->type) {
        case ADDITION:
            if (this->val1->num->num == 0) {
                //adding zero gives second value
                this->type = VALUE;
                this->val1 = this->val2;
            }
            break;
        case SUBTRACTION:
            if (this->val1->num->num == 0) {
                //subtracting from zero so output is always zero
                this->type = VALUE;
            }
            break;
        case MULTIPLICATION:
            if (this->val1->num->num == 0) {
                //multiplication by zero leaves zero
                this->type = VALUE;
            } else if (this->val1->num->num == 1) {
                //multiplication by one leaves only second value
                this->type = VALUE;
                this->val1 = this->val2;
            }
            break;
        case DIVISION:
            if (this->val1->num->num == 0) {
                //division zero by something leaves zero
                this->type = VALUE;
            }
            break;
        case MODULO:
            if (this->val1->num->num == 0) {
                //modulo zero by something leaves zero
                this->type = VALUE;
            } else if (this->val1->num->num == 1) {
                //modulo one by something leaves zero
                this->type = VALUE;
                this->val1->num->num = 0;
            }
            break;
        default:
            break;
    }
}

void Expression::oneConstRightValueOptimizations(){
    //some operations optimisations
    switch(this->type) {
        case ADDITION:
        case SUBTRACTION:
            if (this->val2->num->num == 0) {
                //subtracting and adding zero leaves value1
                this->type = VALUE;
            }
            break;
        case MULTIPLICATION:
            if (this->val2->num->num == 0) {
                //multiplication by 0 leaves always zero
                this->type = VALUE;
                this->val1 = this->val2; //val2 is number 0 already
            } else if (this->val2->num->num == 1) {
                //multiplication by 1 leaves value1
                this->type = VALUE;
            }
            break;
        case DIVISION:
            if (this->val2->num->num == 0) {
                //division by 0 leaves always zero
                this->type = VALUE;
                this->val1 = this->val2; //val2 is number 0 already
            } else if (this->val2->num->num == 1) {
                //division by 1 leaves value1
                this->type = VALUE;
            }
            break;
        case MODULO:
            if (this->val2->num->num == 0) {
                //modulo by 0 leaves always zero
                this->type = VALUE;
                this->val1 = this->val2; //val2 is number 0 already
            } else if (this->val2->num->num == 1) {
                //modulo by 1 leaves always zero
                this->type = VALUE;
                this->val2->num->num = 0; //val2 is number, set to 0
                this->val1 = this->val2; //use val2 as main val
            }
            break;
        default:
            break;
    }
}

void Expression::loadToAccumulatorValue() {
    this->val1->loadToAccumulator();
}

void Expression::loadToAccumulatorAddition() {
    //todo change if NUMBER loading changes
    //addition is commutative and its cheaper to load number first
    //cause adding number to accumulator needs to hold accumulator in temporary variable (LOAD and STORE faster)
    if(this->val2->isLoadBetterThanAdd()){
        this->val2->loadToAccumulator();
        this->val1->addToAccumulator();
    } else {
        this->val1->loadToAccumulator();
        this->val2->addToAccumulator();
    }
/*  this->val1->loadToAccumulator();
    this->val2->addToAccumulator();*/
}

void Expression::loadToAccumulatorSubtraction() {
    this->val1->loadToAccumulator();
    this->val2->subFromAccumulator();
}

void loadToAccumulatorMultiplicationDefault(Value* val1, Value* val2);
void loadToAccumulatorMultiplicationByNumber(Value* valNotNum, Value* valNum);

void Expression::loadToAccumulatorMultiplication() {
    //optimization for number
    if(this->val1->type == Value::Type::NUM || this->val2->type == Value::Type::NUM){
        Value* valNum = val1->type == Value::Type::NUM ? val1 : val2;
        Value* valNotNum = val1->type == Value::Type::NUM ? val2 : val1;

        loadToAccumulatorMultiplicationByNumber(valNotNum, valNum);
    } else {
        loadToAccumulatorMultiplicationDefault(this->val1, this->val2);
    }
}

void loadToAccumulatorMultiplicationDefault(Value* val1, Value* val2){
    auto currentShiftedNumber = memory.pushTempVariable();
    auto bits = memory.pushTempVariable();
    auto result = memory.pushTempVariable();

    auto value1LessOrEqualsValue2 = new JumpPosition();
    auto multiplicationLoopStart = new JumpPosition();
    auto multiplicationEnd = new JumpPosition();
    auto multiplicationAdding = new JumpPosition();
    auto multiplicationShifting = new JumpPosition();


    //multiplication, init result
    machine.ZERO();
    machine.STORE(result->memoryPtr);

    //compare values to chose which one is smaller for faster logarithmic multiplication
    val1->loadToAccumulator();
    val2->subFromAccumulator();
    //val1 <= val2
    machine.JZERO(value1LessOrEqualsValue2);

    //val1 > val2
    val1->loadToAccumulator();
    machine.STORE(currentShiftedNumber->memoryPtr);
    val2->loadToAccumulator();
    machine.STORE(bits->memoryPtr);
    machine.JUMP(multiplicationLoopStart);

    machine.setJumpPosition(value1LessOrEqualsValue2);
    //val1 <= val2
    val2->loadToAccumulator();
    machine.STORE(currentShiftedNumber->memoryPtr);
    val1->loadToAccumulator();
    machine.STORE(bits->memoryPtr);

    //bits from last loop are loaded in accumulator so doesnt need to load again
    machine.setJumpPosition(multiplicationLoopStart);

    //when multiplier with bits is 0 jump to end
    machine.JZERO(multiplicationEnd);
    //optimization - not storing when jump to end
    machine.STORE(bits->memoryPtr);

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

    machine.JUMP(multiplicationLoopStart);

    machine.setJumpPosition(multiplicationEnd);
    machine.LOAD(result->memoryPtr);

    memory.popTempVariable(); //currentShiftedNumber
    memory.popTempVariable(); //bits
    memory.popTempVariable(); //result
}

void loadToAccumulatorMultiplicationByNumber(Value* valNotNum, Value* valNum){
    stack<int> bits = valNum->num->getBits();
    valNotNum->prepareIfNeeded();

    //first bit always 1
    bits.pop();

    valNotNum->loadToAccumulator();

    while(!bits.empty()){
        machine.SHL();

        int bit = bits.top();
        bits.pop();

        if(bit == 1){
            valNotNum->addToAccumulator();
        }
    }
}

void Expression::loadToAccumulatorDivision() {

    //division a/a=1
    if(this->val1->equals(this->val2)){
        machine.ZERO();
        machine.INC();
        return;
    }

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
    auto outsideDivision = new JumpPosition();

    machine.ZERO();
    machine.STORE(result->memoryPtr);
    machine.INC();
    machine.STORE(currentBit->memoryPtr);

    this->val1->loadToAccumulator();
    machine.STORE(currentDividend->memoryPtr);
    //check if have to load
    this->val2->loadToAccumulator();
    machine.STORE(currentDivider->memoryPtr);

    //jump if divider = 0
    machine.JZERO(outsideDivision);
    //optimization, load here or it will be loaded at divider shifting start
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
    machine.SUB(currentDividend->memoryPtr);
    //if divider <= dividend shift divider right
    machine.JZERO(dividerShiftingStart);

    machine.setJumpPosition(divisionShiftingStart);
    //SHIFT CURRENT BIT
    //divider > dividend, so shift once right
    machine.LOAD(currentBit->memoryPtr);
    machine.SHR();

    //dividing if currentBit > 0
    //jump to end if currentBit == 0
    machine.JZERO(divisionEnd);
    machine.STORE(currentBit->memoryPtr);

    //SHIFT CURRENT DIVIDER
    machine.LOAD(currentDivider->memoryPtr);
    machine.SHR();
    machine.STORE(currentDivider->memoryPtr);

    machine.SUB(currentDividend->memoryPtr);
    //divider <= dividend, jump to subtract
    machine.JZERO(divisionSubtraction);
    //divider > dividend
    machine.JUMP(divisionShiftingStart);

    machine.setJumpPosition(divisionSubtraction);
    //subtraction
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
    //edge case, if divider = 0 then result = 0 so jump outside immidiately
    //OPTIMIZATION no need to load result
    machine.setJumpPosition(outsideDivision);

    memory.popTempVariable(); //currentBit
    memory.popTempVariable(); //currentDividend
    memory.popTempVariable(); //currentDivider
    memory.popTempVariable(); //result
}

void Expression::loadToAccumulatorModulo() {
    //TODO optimisations pow 2

    //division a%a=0
    if (this->val1->equals(this->val2)){
        machine.ZERO();
        return;
    }

    Variable* currentDividend = memory.pushTempVariable();
    Variable* currentDivider = memory.pushTempVariable();

    auto firstDividerDividendComparision = new JumpPosition();
    auto dividerShiftingStart = new JumpPosition();
    auto divisionShiftingStart = new JumpPosition();
    auto divisionSubtraction = new JumpPosition();
    auto divisionEnd = new JumpPosition();
    auto outsideDivision = new JumpPosition();

    this->val1->loadToAccumulator();
    machine.STORE(currentDividend->memoryPtr);
    this->val2->loadToAccumulator();
    machine.STORE(currentDivider->memoryPtr);

    //jump if divider = 0
    machine.JZERO(outsideDivision);

    machine.JUMP(firstDividerDividendComparision);

    //shift current bit and divider, and to divider must be <= divident
    machine.setJumpPosition(dividerShiftingStart);
    machine.LOAD(currentDivider->memoryPtr);
    machine.SHL();
    machine.STORE(currentDivider->memoryPtr);

    //jumping here first time
    machine.setJumpPosition(firstDividerDividendComparision);
    //currentDivider already in accumulator - optimization
    machine.SUB(currentDividend->memoryPtr);
    //if divider <= dividend shift divider right
    machine.JZERO(dividerShiftingStart);

    machine.setJumpPosition(divisionShiftingStart);
    //divider reached starting value end sub = 0 so end
    machine.LOAD(currentDivider->memoryPtr);
    this->val2->subFromAccumulator();

    //dividing if currentBit > 0
    //jump to end if currentBit == 0
    machine.JZERO(divisionEnd);

    //SHIFT CURRENT DIVIDER
    machine.LOAD(currentDivider->memoryPtr);
    machine.SHR();
    machine.STORE(currentDivider->memoryPtr);

    machine.SUB(currentDividend->memoryPtr);
    //divider <= dividend, jump to subtract
    machine.JZERO(divisionSubtraction);
    //divider > dividend
    machine.JUMP(divisionShiftingStart);

    machine.setJumpPosition(divisionSubtraction);
    //subtraction
    machine.LOAD(currentDividend->memoryPtr);
    machine.SUB(currentDivider->memoryPtr);
    machine.STORE(currentDividend->memoryPtr);
    //jump to shifting start
    machine.JUMP(divisionShiftingStart);

    machine.setJumpPosition(divisionEnd);
    machine.LOAD(currentDividend->memoryPtr);
    //edge case, if divider = 0 then result = 0 so jump outside immidiately
    //loading divident will make error
    machine.setJumpPosition(outsideDivision);

    memory.popTempVariable(); //currentDividend
    memory.popTempVariable(); //currentDivider
}


