#pragma once

#include "Command.h"

class Command;

class CommandsBlock {
public:
    vector<Command*> commands;

    void addCommand(Command* command){
        commands.push_back(command);
    }
};
