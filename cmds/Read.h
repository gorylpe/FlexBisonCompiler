#pragma once

#include "../Command.h"
#include "../IdentifiersSSAHelper.h"
#include "../IdentifiersUsagesHelper.h"

class Read : public Command {
public:
    Identifier* ident;

    explicit Read(Identifier* ident)
            :ident(ident){
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Creating " << toString() << endl;
    #endif
    }

    Read(const Read& read2)
            :ident(read2.ident->clone()){}

    Read* clone() const final {
        return new Read(*this);
    }

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }

        cerr << toString() << endl;
    }

    string toString() final {
        return "READ " + ident->toString() + ";";
    }

    void semanticAnalysis() final {
        ident->semanticAnalysisSet();
    }

    void generateCode() final{
        cerr << "Generating " << toString() << endl;
        this->ident->prepareIfNeeded();

        machine.GET();
        ident->storeFromAccumulator();

        this->ident->unprepareIfNeeded();
    }

    void simplifyExpressions() final {}

    void collectUsagesData(IdentifiersUsagesHelper &stats) final {}

    CommandsBlock* blockToReplaceWith() final {
        return nullptr;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->ident->calculateVariablesUsage(numberOfNestedLoops);
    }

    void calculateSSANumbersInIdentifiers(IdentifiersSSAHelper &prevStats) final {
        prevStats.setForStore(ident);
    }
};