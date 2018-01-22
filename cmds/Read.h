#pragma once

#include "../Command.h"

class Read : public Command {
public:
    Identifier* ident;

    explicit Read(Identifier* ident)
            :ident(ident){
        cerr << "Creating " << toString() << endl;
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

    CommandsBlock* blockToReplaceWith(){
        return nullptr;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->ident->calculateVariablesUsage(numberOfNestedLoops);
    }

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        Variable* var = this->ident->getPidVariable();
        if(var->type == Variable::Type::PID)
            variableSet.insert(var);
    }
};