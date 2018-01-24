#pragma once

#include "../Command.h"
#include "../IdentifiersSSAHelper.h"
#include "../IdentifiersUsagesHelper.h"
#include "Assignment.h"

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
        #ifdef DEBUG_LOG_GENERATING_CODE
        cerr << "Generating " << toString() << endl;
        #endif
        this->ident->prepareIfNeeded();

        machine.GET();
        ident->storeFromAccumulator();

        this->ident->unprepareIfNeeded();
    }

    void simplifyExpressions() final {}

    void collectUsagesData(IdentifiersUsagesHelper &stats) final {}

    int searchUnusedAssignmentsAndSetForDeletion(IdentifiersUsagesHelper &helper) final { return 0; }

    void collectAssignmentsForIdentifiers(IdentifiersAssignmentsHelper& helper) final {}

    int propagateValues(IdentifiersAssignmentsHelper &assgnsHelper, IdentifiersUsagesHelper &usagesHelper) final {
        int propagated = 0;
        if(ident->isTypePIDPID()){
            stringstream ss;
            if(pflags.verbose()) {
                ss << "PROPAGATED" << endl;
                ss << toString() << endl;
                ss << " TO " << endl;
            }

            if(Assignment::tryToPropagatePidpid(assgnsHelper, usagesHelper, *ident)){
                ++propagated;
            }

            if(pflags.verbose()){
                if(propagated > 0){
                    ss << toString() << endl;
                    cerr << ss.str();
                }
            }
        }
        return propagated;
    }

    CommandsBlock* blockToReplaceWith() final {
        return nullptr;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->ident->calculateVariablesUsage(numberOfNestedLoops);
    }

    void collectSSANumbersInIdentifiers(IdentifiersSSAHelper &prevStats) final {
        prevStats.setForStore(ident);
    }
};