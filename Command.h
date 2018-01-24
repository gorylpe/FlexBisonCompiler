#pragma once

#include <cln/cln.h>
#include <vector>
#include <sstream>
#include <typeinfo>
#include "Identifier.h"
#include "Condition.h"
#include "NumberValueStats.h"
#include "IdentifiersUsagesHelper.h"

using namespace std;
using namespace cln;

class CommandsBlock;
class IdentifiersSSAHelper;

class Command {
public:
    virtual void semanticAnalysis() = 0;

    virtual void generateCode() = 0;

    virtual bool equals(Command *command) {
        return false;
    }

    virtual Command* clone() const = 0;

    virtual string toString() = 0;

    virtual void print(int nestedLevel) = 0;

    virtual void calculateVariablesUsage(cl_I numberOfNestedLoops) = 0;

    virtual void simplifyExpressions() = 0;

    //used to remove not reachable ifs, unroll for's with constants
    virtual CommandsBlock* blockToReplaceWith() = 0;

    virtual void replaceCommands() {}

    //for FOR unrolling - replacing iterator with consts
    virtual void replaceValuesWithConst(string pid, cl_I number) {}

    virtual void calculateSSANumbersInIdentifiers(IdentifiersSSAHelper &helper) = 0;

    virtual void collectUsagesData(IdentifiersUsagesHelper &helper) = 0;

    //for creating numbers optimal
    virtual void collectNumberValues(map<cl_I, NumberValueStats>& stats) {}
};

class CommandsBlock : public Command {
public:
    vector<Command*> commands;

    CommandsBlock() = default;

    CommandsBlock(const CommandsBlock& block2){
        for(auto cmd : block2.commands){
            addCommand(cmd->clone());
        }
    }

    CommandsBlock* clone() const final {
        return new CommandsBlock(*this);
    }

    void addCommand(Command* command){
        commands.push_back(command);
    }

    void addCommands(vector<Command*>& newCommands){
        commands.insert(commands.end(), newCommands.begin(), newCommands.end());
    }

    void print(int nestedLevel) final {
        for(auto cmd : commands){
            cmd->print(nestedLevel);
        }
    }

    string toString() final {return "";}

    bool equals(Command* command) final {
        auto block2 = dynamic_cast<CommandsBlock*>(command);

        if(block2 == nullptr)
            return false;

        if(this->commands.size() != block2->commands.size())
            return false;

        for(int i = 0; i < this->commands.size(); ++i){
            if(!this->commands[i]->equals(block2->commands[i]))
                return false;
        }

        return true;
    }

    void semanticAnalysis() final {
        for(auto cmd : this->commands){
            //cerr << "Semantic analysis for " << typeid(*cmd).name() << endl;
            cmd->semanticAnalysis();
        }
    }

    void generateCode() final {
        for(auto cmd : this->commands){
            cmd->generateCode();
        }
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        for(auto cmd : this->commands){
            cmd->calculateVariablesUsage(numberOfNestedLoops);
        }
    }

    void simplifyExpressions() final {
        for(auto cmd : this->commands){
            cmd->simplifyExpressions();
        }
    }

    void collectUsagesData(IdentifiersUsagesHelper &helper) final {
        bool hasPropagated = false;

        for(auto cmd : this->commands){
            cerr << "COLLECTING USAGES DATA FOR " << typeid(*cmd).name() << endl;
            cmd->collectUsagesData(helper);
        }
    }

    CommandsBlock* blockToReplaceWith() final {
        return this;
    }

    void replaceCommands() final {
        for(int i = 0; i < commands.size(); ++i) {
            CommandsBlock* block = commands[i]->blockToReplaceWith();

            if (block != nullptr) {
                block->replaceCommands();
                commands.erase(commands.begin() + i);
                commands.insert(commands.begin() + i, block->commands.begin(), block->commands.end());
                i--;
                i += block->commands.size();
            } else {
                commands[i]->replaceCommands();
            }
        }
    }

    void replaceValuesWithConst(string pid, cl_I number) final {
        for(auto cmd : this->commands){
            cmd->replaceValuesWithConst(pid, number);
        }
    }

    void calculateSSANumbersInIdentifiers(IdentifiersSSAHelper &prevStats) final {
        for(auto cmd : this->commands){
            //cerr << "STATS FOR " << typeid(*cmd).name() << endl;
            cmd->calculateSSANumbersInIdentifiers(prevStats);
        }
    };

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        for(auto cmd : this->commands){
            cmd->collectNumberValues(stats);
        }
    }
};

