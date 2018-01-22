//
// Created by piotr on 17.12.2017.
//

#include "Expression.h"

Expression::Expression(Value *val)
        :type(VALUE)
        ,val1(val){
    cerr << "Creating expression " << toString() << endl;
}

Expression::Expression(Expression::Type type, Value *val1, Value *val2)
        :type(type)
        ,val1(val1)
        ,val2(val2){
    //const operation optimisation
    this->optimizeConstants();

    cerr << "Creating expression " << toString() << endl;
}

void Expression::optimizeConstants() {
    //2 values
    if(this->type != VALUE){
        this->bothConstValuesOptimizations();
        this->oneConstLeftValueOptimizations();
        this->oneConstRightValueOptimizations();
        this->addingNumberOptimization();
    }
}

void Expression::bothConstValuesOptimizations() {
    if(val1->type == Value::Type::NUM && val2->type == Value::Type::NUM) {
        switch (this->type) {
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
                if (val2->num->num == 0) {
                    val1->num->num = 0;
                } else {
                    val1->num->num = floor1(val1->num->num / val2->num->num);
                }
                break;
            case MODULO:
                if (val2->num->num == 0) {
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
}

void Expression::oneConstLeftValueOptimizations(){
    //some operations optimisations
    if (val1->type == Value::Type::NUM && val2->type == Value::Type::IDENTIFIER) {
        switch (this->type) {
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
}

void Expression::oneConstRightValueOptimizations(){
    //some operations optimisations
    if (val2->type == Value::Type::NUM && val1->type == Value::Type::IDENTIFIER) {
        switch (this->type) {
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
}

void Expression::addingNumberOptimization() {
    if(type == ADDITION){
        if(val2->type == Value::Type::NUM){
            if(val2->isLoadBetterThanIncs()){
                Value* tmp = val1;
                val1 = val2;
                val2 = tmp;
            }
        }
    }
}

void Expression::loadToAccumulatorValue() {
    this->val1->loadToAccumulator();
}

void Expression::loadToAccumulatorAddition() {
    //addition a+a = 2*a = a << 1
    if(this->val1->equals(this->val2)){
        this->val1->loadToAccumulator();
        machine.SHL();
        return;
    }

    //addition is commutative and its cheaper to load number first
    //cause adding number to accumulator needs to hold accumulator in temporary variable (LOAD and STORE faster)
    this->val1->loadToAccumulator();
    this->val2->addToAccumulator();
}

void Expression::loadToAccumulatorSubtraction() {
    //subtraction a-a=0
    if(this->val1->equals(this->val2)){
        machine.ZERO();
        return;
    }

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
    //when multiplying by 0 result is 0
    if(valNum->num->num == 0){
        machine.ZERO();
        return;
    }

    stack<int> bits = Number::getBits(valNum->num->num);

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

void loadToAccumulatorDivisionDefault(Value* val1, Value* val2);
void loadToAccumulatorDivisionByNumber(Value* val1Ident, Value* val2Num);

void Expression::loadToAccumulatorDivision() {
    //optimization for number
    if(this->val1->type == Value::Type::IDENTIFIER && this->val2->type == Value::Type::NUM){
        loadToAccumulatorDivisionByNumber(this->val1, this->val2);
    } else {
        loadToAccumulatorDivisionDefault(this->val1, this->val2);
    }
}

//optimization for powers of 2
void loadToAccumulatorDivisionByNumber(Value* val1Ident, Value* val2Num){
    //when dividing by 0 result is 0
    if(val2Num->num->num == 0){
        machine.ZERO();
        return;
    }
    //when dividing by 1 result is 1
    if(val2Num->num->num == 1){
        val1Ident->loadToAccumulator();
        return;
    }

    if(Expression::isPowOf2(val2Num->num->num)){
        val1Ident->loadToAccumulator();

        int shifts = Number::getBits(val2Num->num->num).size() - 1;
        for(int i = 0; i < shifts; ++i){
            machine.SHR();
        }
    } else {
        loadToAccumulatorDivisionDefault(val1Ident, val2Num);
    }
}

void loadToAccumulatorDivisionDefault(Value* val1, Value* val2) {
    //division a/a=1
    if(val1->equals(val2)){
        machine.ZERO();
        machine.INC();
        return;
    }

    auto currentBit = memory.pushTempVariable();
    auto currentDividend = memory.pushTempVariable();
    auto currentDivider = memory.pushTempVariable();
    auto result = memory.pushTempVariable();

    auto dividerShiftingStart = new JumpPosition();
    auto divisionShiftingStart = new JumpPosition();
    auto divisionSubtraction = new JumpPosition();
    auto divisionEnd = new JumpPosition();
    auto outsideDivision = new JumpPosition();

    machine.ZERO();
    machine.STORE(result->memoryPtr);
    machine.INC();
    machine.STORE(currentBit->memoryPtr);

    val1->loadToAccumulator();
    machine.STORE(currentDividend->memoryPtr);
    //check if have to load
    val2->loadToAccumulator();
    machine.STORE(currentDivider->memoryPtr);

    //jump if divider = 0
    machine.JZERO(outsideDivision);

    //optimization, most of the time divider is much less than divident, can do shift left instantly and 4 times at row (tested empirically)
    //shift current bit and divider, and to divider must be <= divident
    machine.setJumpPosition(dividerShiftingStart);
    machine.LOAD(currentBit->memoryPtr);
    machine.SHL();
    machine.SHL();
    machine.SHL();
    machine.SHL();
    machine.STORE(currentBit->memoryPtr);
    machine.LOAD(currentDivider->memoryPtr);
    machine.SHL();
    machine.SHL();
    machine.SHL();
    machine.SHL();
    machine.STORE(currentDivider->memoryPtr);
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


void loadToAccumulatorModuloDefault(Value* val1, Value* val2);
void loadToAccumulatorModuloByNumber(Value* val1Ident, Value* val2Num);

void Expression::loadToAccumulatorModulo() {
    //optimization for number
    if(this->val1->type == Value::Type::IDENTIFIER && this->val2->type == Value::Type::NUM){
        loadToAccumulatorModuloByNumber(this->val1, this->val2);
    } else {
        loadToAccumulatorModuloDefault(this->val1, this->val2);
    }
}

//optimization for 2
void loadToAccumulatorModuloByNumber(Value* val1Ident, Value* val2Num){
    //when modulo by 0 result is 0
    if(val2Num->num->num == 0){
        machine.ZERO();
        return;
    }
    //when modulo by 1 result is 0
    if(val2Num->num->num == 1){
        machine.ZERO();
        return;
    }
    if(val2Num->num->num == 2){
        val1Ident->loadToAccumulator();

        auto one = new JumpPosition();
        auto outside = new JumpPosition();

        machine.JODD(one);
        machine.ZERO();
        machine.JUMP(outside);
        machine.setJumpPosition(one);
        machine.ZERO();
        machine.INC();
        machine.setJumpPosition(outside);
    } else {
        loadToAccumulatorModuloDefault(val1Ident, val2Num);
    }
}

void loadToAccumulatorModuloDefault(Value* val1, Value* val2) {
    //division a%a=0
    if (val1->equals(val2)){
        machine.ZERO();
        return;
    }

    Variable* currentDividend = memory.pushTempVariable();
    Variable* currentDivider = memory.pushTempVariable();

    auto firstDividerDividendComparision = new JumpPosition();
    auto dividerShiftingStart = new JumpPosition();
    auto dividerShiftingStartWithoutDividerLoad = new JumpPosition();
    auto divisionShiftingStart = new JumpPosition();
    auto divisionShiftingStartWithDividendLoad = new JumpPosition();
    auto divisionSubtraction = new JumpPosition();
    auto divisionEnd = new JumpPosition();
    auto outsideDivision = new JumpPosition();

    val1->loadToAccumulator();
    machine.STORE(currentDividend->memoryPtr);
    val2->loadToAccumulator();
    machine.STORE(currentDivider->memoryPtr);

    //jump if divider = 0
    machine.JZERO(outsideDivision);
    machine.JUMP(dividerShiftingStartWithoutDividerLoad);

    //optimization, most of the time divider is much less than divident, can do shift left instantly and 3 times at row (tested empirically)
    //shift current bit and divider, and to divider must be <= divident
    machine.setJumpPosition(dividerShiftingStart);
    machine.LOAD(currentDivider->memoryPtr);
    machine.setJumpPosition(dividerShiftingStartWithoutDividerLoad);
    machine.SHL();
    machine.SHL();
    machine.SHL();
    machine.STORE(currentDivider->memoryPtr);

    //jumping here first time
    machine.setJumpPosition(firstDividerDividendComparision);
    //currentDivider already in accumulator - optimization
    machine.SUB(currentDividend->memoryPtr);
    //if divider <= dividend shift divider right
    machine.JZERO(dividerShiftingStart);

    //cmp currentDivident + 1 <= divisor -> currentDivident  < divisor so its RESULT, mod < divisor
    machine.setJumpPosition(divisionShiftingStartWithDividendLoad);
    machine.LOAD(currentDividend->memoryPtr);
    machine.setJumpPosition(divisionShiftingStart);
    machine.INC();
    val2->subFromAccumulator();
    machine.JZERO(divisionEnd);

    //SHIFT CURRENT DIVIDER
    machine.LOAD(currentDivider->memoryPtr);
    machine.SHR();
    machine.STORE(currentDivider->memoryPtr);

    //cmp current divider <= current dividend
    machine.SUB(currentDividend->memoryPtr);
    machine.JZERO(divisionSubtraction);
    machine.JUMP(divisionShiftingStartWithDividendLoad);

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
