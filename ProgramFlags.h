#pragma once

#define DEFUALT_WHILE_UNROLLS_NUMBER 50

class ProgramFlags{
    ProgramFlags():whileUnrollsNumber(DEFUALT_WHILE_UNROLLS_NUMBER)
    ,unrollFors(true)
    ,optimizeSimilarNumbers(true)
    ,propagateValues(true)
    ,memoryOptimization(true)
    ,verboseOutput(true){};

    int whileUnrollsNumber;
    bool unrollFors;
    bool optimizeSimilarNumbers;
    bool propagateValues;
    bool memoryOptimization;
    bool verboseOutput;

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
        unrollFors = newVal;
    }

    void setOptimizeSimilarNumbers(bool newVal){
        optimizeSimilarNumbers = newVal;
    }

    void setPropagateValues(bool newVal){
        propagateValues = newVal;
    }

    void setMemoryOptimization(bool newVal){
        memoryOptimization = newVal;
    }

    void setVerbose(bool newVal){
        verboseOutput = newVal;
    }

    int getWhileUnrollsNumber(){
        return whileUnrollsNumber;
    }

    bool getUnrollFors(){
        return unrollFors;
    }

    bool getOptimizeSimilarNumbers(){
        return optimizeSimilarNumbers;
    }

    bool getPropagateValues(){
        return propagateValues;
    }

    bool getMemoryOptimization(){
        return memoryOptimization;
    }

    bool verbose(){
        return verboseOutput;
    }
};