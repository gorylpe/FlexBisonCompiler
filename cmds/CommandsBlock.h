#pragma once

#include "../Command.h"

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

    bool propagateConstants() final {
        bool hasPropagated = false;

        for(auto cmd : this->commands){
            if(cmd->propagateConstants())
                hasPropagated = true;
        }

        return hasPropagated;
    }

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        for(auto cmd : this->commands){
            cmd->getPidVariablesBeingModified(variableSet);
        }
    }

    CommandsBlock* blockToReplaceWith(){
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

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        for(auto cmd : this->commands){
            cmd->collectNumberValues(stats);
        }
    }
};
