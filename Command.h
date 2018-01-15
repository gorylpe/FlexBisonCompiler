#pragma once

#include <cln/cln.h>
#include <vector>
#include <sstream>
#include "Identifier.h"
#include "Expression.h"
#include "Condition.h"
#include "CommandsBlock.h"

using namespace std;
using namespace cln;

class Command {
public:
    virtual void generateCode() = 0;
};

class Assignment : public Command {
public:
    Identifier* ident;
    Expression* expr;

    explicit Assignment(Identifier* ident, Expression* expr)
    :ident(ident)
    ,expr(expr){
        cerr << "Creating ASSIGNMENT with " << ident->toString() << ", " << expr->toString() << endl;
    }

    virtual void generateCode(){
        cerr << "Generating ASSIGNMENT code" << endl;
        this->expr->loadToAccumulator();
        this->ident->storeFromAccumulator();
        cerr << "End generating ASSIGNMENT code" << endl;
    }
};

class Read : public Command {
public:
    Identifier* ident;

    explicit Read(Identifier* ident)
            :ident(ident){
        cerr << "Creating READ with " << ident->toString() << endl;
    }

    void generateCode(){
        cerr << "Generating READ code" << endl;
        machine.GET();
        ident->storeFromAccumulator();
        cerr << "End generating READ code" << endl;
    }
};

class Write : public Command {
public:
    Value* val;

    explicit Write(Value* val)
    :val(val){
        cerr << "Creating WRITE with " << val->toString() << endl;
    }

    void generateCode(){
        cerr << "Generating WRITE code" << endl;
        this->val->loadToAccumulator();
        machine.PUT();
        cerr << "End generating WRITE code" << endl;
    }
};

class If : public Command{
public:
    Condition* cond;
    CommandsBlock* block;

    If(Condition* cond, CommandsBlock* block)
    :cond(cond)
    ,block(block){
        cerr << "Creating IF with " << cond->toString() << endl;
    }

    virtual void generateCode() {
        cerr << "Generating IF code" << endl;
        //TODO check if num of commands > 0
        if(!this->cond->constComparision){
            auto jumpIfTrue = new JumpPosition();
            auto jumpIfFalse = new JumpPosition();

            this->cond->generateTestWithTrueFalseJumps(jumpIfTrue, jumpIfFalse);
            machine.setJumpPosition(jumpIfTrue);
            for(auto cmd : this->block->commands){
                cmd->generateCode();
            }
            machine.setJumpPosition(jumpIfFalse);
        } else if (this->cond->constComparisionResult){
            for(auto cmd : this->block->commands){
                cmd->generateCode();
            }
        }
        cerr << "End generating IF code" << endl;
    }
};

class IfElse : public Command{
public:
    Condition* cond;
    CommandsBlock* block1;
    CommandsBlock* block2;

    IfElse(Condition* cond, CommandsBlock* block1, CommandsBlock* block2)
            :cond(cond)
            ,block1(block1)
            ,block2(block2){
        cerr << "Creating IF with " << cond->toString() << endl;
    }

    virtual void generateCode() {
        cerr << "Generating IF ELSE code" << endl;
        //TODO check if num of commands > 0
        if(!this->cond->constComparision){
            auto jumpIfTrue = new JumpPosition();
            auto jumpIfFalse = new JumpPosition();
            auto passElseBlock = new JumpPosition();

            this->cond->generateTestWithTrueFalseJumps(jumpIfTrue, jumpIfFalse); //jump to else

            machine.setJumpPosition(jumpIfTrue);
            for(auto cmd : this->block1->commands){
                cmd->generateCode();
            }

            machine.JUMP(passElseBlock);

            machine.setJumpPosition(jumpIfFalse);
            for(auto cmd : this->block2->commands){
                cmd->generateCode();
            }

            machine.setJumpPosition(passElseBlock);

        } else if (this->cond->constComparisionResult){
            for(auto cmd : this->block1->commands){
                cmd->generateCode();
            }
        } else {
            for(auto cmd : this->block2->commands){
                cmd->generateCode();
            }
        }
        cerr << "End generating IF ELSE code" << endl;
    }
};

class While : public Command{
public:
    Condition* cond;
    CommandsBlock* block;

    While(Condition* cond, CommandsBlock* block)
    :cond(cond)
    ,block(block){
        cerr << "Creating WHILE with " << cond->toString() << endl;
    }

    virtual void generateCode() {
        cerr << "Generating WHILE code" << endl;
        //TODO check if num of commands > 0
        if(!this->cond->constComparision){
            auto loopStart = new JumpPosition();
            auto codeStart = new JumpPosition();
            auto loopOutside = new JumpPosition();

            machine.setJumpPosition(loopStart);

            this->cond->generateTestWithTrueFalseJumps(codeStart, loopOutside);

            machine.setJumpPosition(codeStart);
            for(auto cmd : this->block->commands){
                cmd->generateCode();
            }

            machine.JUMP(loopStart);

            machine.setJumpPosition(loopOutside);

        } else if (this->cond->constComparisionResult) {
            auto loopStart = new JumpPosition();
            machine.setJumpPosition(loopStart);

            for(auto cmd : this->block->commands){
                cmd->generateCode();
            }

            machine.JUMP(loopStart);
        }
        cerr << "End generating WHILE code" << endl;
    }
};

class For : public Command{
public:

    Position* pidPos;
    const string& pid;
    Value* from;
    Value* to;
    CommandsBlock* block;
    bool increasing;

    explicit For(Position* pidPos, const string& pid, Value* from, Value* to, CommandsBlock* block, bool increasing)
    :pid(pid)
    ,pidPos(pidPos)
    ,from(from)
    ,to(to)
    ,block(block)
    ,increasing(increasing){
        cerr << "Creating FOR" << endl;
    }

    virtual void generateCode(){
        cerr << "Generating FOR code" << endl;
        auto iterator = memory.pushTempNamedVariable(pid, false);
        auto tmpTo = memory.pushTempVariable();

        auto loopStart = new JumpPosition();
        auto loopOutside = new JumpPosition();

        if(iterator == nullptr){
            poserror(pidPos, "for loop temp variable redeclaration");
            return;
        }

        this->from->loadToAccumulator();
        machine.STORE(iterator->memoryPtr);
        iterator->initialize();
        this->to->loadToAccumulator();
        machine.STORE(tmpTo->memoryPtr);
        tmpTo->initialize();

        //first comparision - first check needed for initial condition
        //edge case - if going DOWNTO 0 cant go -1, need checking after loop if equals before iterator inc/dec
        if(!this->increasing){
            machine.LOAD(iterator->memoryPtr);
            machine.INC();
            machine.SUB(tmpTo->memoryPtr);
            machine.JZERO(loopOutside);
        }

        machine.setJumpPosition(loopStart);

        for(auto cmd : this->block->commands){
            cmd->generateCode();
        }

        if(this->increasing){ //0 when iterator >= to
            machine.LOAD(tmpTo->memoryPtr);
            machine.SUB(iterator->memoryPtr);
        } else { // 0 when iterator <= to
            machine.LOAD(iterator->memoryPtr);
            machine.SUB(tmpTo->memoryPtr);
        }

        machine.JZERO(loopOutside);

        // update iterator
        machine.LOAD(iterator->memoryPtr);
        if(this->increasing)
            machine.INC();
        else
            machine.DEC();
        machine.STORE(iterator->memoryPtr);

        machine.JUMP(loopStart);

        machine.setJumpPosition(loopOutside);

        memory.popTempVariable(); //iterator
        memory.popTempVariable(); //tmpTo

        cerr << "End generating FOR code" << endl;
    }
};

