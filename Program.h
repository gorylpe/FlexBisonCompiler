#pragma once

#include <sstream>
#include "Command.h"
#include "CommandsBlock.h"

class Program {
public:
    stringstream ss;
    CommandsBlock* block;

    void setBlock(CommandsBlock* block){
        this->block = block;
    }

    string generateCode(){
        for(auto cmd : this->block->commands){
            cmd->generateCode();
        }
        machine.HALT();

        machine.generateCode(ss);

        return ss.str();
    }
};
