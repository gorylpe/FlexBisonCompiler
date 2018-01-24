#pragma once

#define DEFUALT_WHILE_UNROLLS_NUMBER 50

class ProgramFlags{
    ProgramFlags():whileUnrollsNumber(DEFUALT_WHILE_UNROLLS_NUMBER)
    ,forsUnrolling(true)
    ,optimizingSimilarNumbers(true)
    ,propagatingValues(true)
    ,memoryOptimization(true)
    ,verboseOutput(false)
    ,showingOptimizedCode(true){};

    int whileUnrollsNumber;
    bool forsUnrolling;
    bool optimizingSimilarNumbers;
    bool propagatingValues;
    bool memoryOptimization;
    bool verboseOutput;
    bool showingOptimizedCode;

public:
    static ProgramFlags& getInstance()
    {
        static ProgramFlags instance;
        return instance;
    }
    ProgramFlags(ProgramFlags const&) = delete;
    void operator=(ProgramFlags const&) = delete;


    void setWhileUnrollsNumber(int newNum){
        whileUnrollsNumber = newNum;
    }

    void setUnrollFors(bool newVal){
        forsUnrolling = newVal;
    }

    void setOptimizeSimilarNumbers(bool newVal){
        optimizingSimilarNumbers = newVal;
    }

    void setPropagateValues(bool newVal){
        propagatingValues = newVal;
    }

    void setMemoryOptimization(bool newVal){
        memoryOptimization = newVal;
    }

    void setVerbose(bool newVal){
        verboseOutput = newVal;
    }

    void setShowOptimizedCode(bool newVal){
        showingOptimizedCode = newVal;
    }

    int getWhileUnrollsNumber(){
        return whileUnrollsNumber;
    }

    bool unrollFors(){
        return forsUnrolling;
    }

    bool optimizeSimilarNumbers(){
        return optimizingSimilarNumbers;
    }

    bool propagateValues(){
        return propagatingValues;
    }

    bool optimizeMemory(){
        return memoryOptimization;
    }

    bool verbose(){
        return verboseOutput;
    }

    bool showOptimizedCode(){
        return showingOptimizedCode;
    }
};