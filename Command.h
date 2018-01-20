#pragma once

#include <cln/cln.h>
#include <vector>
#include <sstream>
#include "Identifier.h"
#include "Expression.h"
#include "Condition.h"

using namespace std;
using namespace cln;

class Command {
public:
    virtual void generateCode() = 0;

    virtual bool equals(Command* command){
        return false;
    }

    virtual void calculateVariablesUsage(cl_I numberOfNestedLoops) = 0;
};

class CommandsBlock : public Command {
public:
    vector<Command*> commands;

    void addCommand(Command* command){
        commands.push_back(command);
    }


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

    void generateCode() final {
        cerr << "Generating ASSIGNMENT code" << endl;
        this->ident->prepareIfNeeded();
        this->expr->prepareValuesIfNeeded();

        this->expr->loadToAccumulator();
        this->ident->storeFromAccumulator();

        this->ident->unprepareIfNeeded();
        this->expr->unprepareValuesIfNeeded();
        cerr << "End generating ASSIGNMENT code" << endl;
    }


    bool equals(Command* command) final {
        auto assgn2 = dynamic_cast<Assignment*>(command);
        if(assgn2 == nullptr)
            return false;

        if(!this->ident->equals(assgn2->ident))
            return false;

        if(!this->expr->equals(assgn2->expr))
            return false;

        return true;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        cerr << "Calculating Variables usage in ASSIGNMENT" << endl;
        this->ident->calculateVariablesUsage(numberOfNestedLoops);
        this->expr->calculateVariablesUsage(numberOfNestedLoops);
        cerr << "End alculating Variables usage in ASSIGNMENT" << endl;
    }
};

class Read : public Command {
public:
    Identifier* ident;

    explicit Read(Identifier* ident)
            :ident(ident){
        cerr << "Creating READ with " << ident->toString() << endl;
    }

    void generateCode() final{
        cerr << "Generating READ code" << endl;
        this->ident->prepareIfNeeded();

        machine.GET();
        ident->storeFromAccumulator();

        this->ident->unprepareIfNeeded();
        cerr << "End generating READ code" << endl;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->ident->calculateVariablesUsage(numberOfNestedLoops);
    }
};

class Write : public Command {
public:
    Value* val;

    explicit Write(Value* val)
    :val(val){
        cerr << "Creating WRITE with " << val->toString() << endl;
    }

    void generateCode() final{
        cerr << "Generating WRITE code" << endl;
        if(this->val->type != Value::Type::NUM)
            this->val->prepareIfNeeded();

        this->val->loadToAccumulator();
        machine.PUT();

        if(this->val->type != Value::Type::NUM)
            this->val->unprepareIfNeeded();
        cerr << "End generating WRITE code" << endl;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->val->calculateVariablesUsage(numberOfNestedLoops);
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

    void generateCode() final {
        cerr << "Generating IF code" << endl;
        //TODO check if num of commands > 0
        if(!this->cond->constComparision){
            this->cond->prepareValuesIfNeeded();

            auto jumpIfTrue = new JumpPosition();
            auto jumpIfFalse = new JumpPosition();

            this->cond->generateTestWithTrueFalseJumps(jumpIfTrue, jumpIfFalse);
            machine.setJumpPosition(jumpIfTrue);

            block->generateCode();

            machine.setJumpPosition(jumpIfFalse);

            this->cond->unprepareValuesIfNeeded();
        } else if (this->cond->constComparisionResult){
            for(auto cmd : this->block->commands){
                cmd->generateCode();
            }
        }
        cerr << "End generating IF code" << endl;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->cond->calculateVariablesUsage(numberOfNestedLoops);
        this->block->calculateVariablesUsage(numberOfNestedLoops);
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

    void generateCode() final {
        //optimization when blocks equals
        if(this->block1->equals(this->block2)){
            cerr << "Optimizing IF ELSE code, same blocks in IF and ELSE" << endl;

            for(auto cmd : this->block1->commands){
                cmd->generateCode();
            }

            cerr << "End optimizing IF ELSE code" << endl;
            return;
        }

        cerr << "Generating IF ELSE code" << endl;
        if(!this->cond->constComparision){
            this->cond->prepareValuesIfNeeded();

            auto jumpIfTrue = new JumpPosition();
            auto jumpIfFalse = new JumpPosition();
            auto passElseBlock = new JumpPosition();

            this->cond->generateTestWithTrueFalseJumps(jumpIfTrue, jumpIfFalse); //jump to else

            machine.setJumpPosition(jumpIfTrue);

            block1->generateCode();

            machine.JUMP(passElseBlock);

            machine.setJumpPosition(jumpIfFalse);

            block2->generateCode();

            machine.setJumpPosition(passElseBlock);

            this->cond->unprepareValuesIfNeeded();
        } else if (this->cond->constComparisionResult){
            block1->generateCode();
        } else {
            block2->generateCode();
        }
        cerr << "End generating IF ELSE code" << endl;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->cond->calculateVariablesUsage(numberOfNestedLoops);
        this->block1->calculateVariablesUsage(numberOfNestedLoops);
        this->block2->calculateVariablesUsage(numberOfNestedLoops);
    }
};

class While : public Command {
public:
    Condition* cond;
    CommandsBlock* block;

    While(Condition* cond, CommandsBlock* block)
    :cond(cond)
    ,block(block){
        cerr << "Creating WHILE with " << cond->toString() << endl;
    }

    void generateCode() final {
        cerr << "Generating WHILE code" << endl;
        if(!this->cond->constComparision){
            this->cond->prepareValuesIfNeeded();

            auto loopStart = new JumpPosition();
            auto codeStart = new JumpPosition();
            auto loopOutside = new JumpPosition();

            machine.setJumpPosition(loopStart);

            this->cond->generateTestWithTrueFalseJumps(codeStart, loopOutside);

            machine.setJumpPosition(codeStart);

            block->generateCode();

            machine.JUMP(loopStart);

            machine.setJumpPosition(loopOutside);

            this->cond->unprepareValuesIfNeeded();
        } else if (this->cond->constComparisionResult) {
            auto loopStart = new JumpPosition();
            machine.setJumpPosition(loopStart);

            block->generateCode();

            machine.JUMP(loopStart);
        }
        cerr << "End generating WHILE code" << endl;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->cond->calculateVariablesUsage(numberOfNestedLoops + 1);
        this->block->calculateVariablesUsage(numberOfNestedLoops + 1);
    }
};

class For : public Command {
public:

    Position* pidPos;
    const string pid;
    Value* from;
    Value* to;
    CommandsBlock* block;
    bool increasing;

    explicit For(Position* pidPos, string pid, Value* from, Value* to, CommandsBlock* block, bool increasing)
    :pid(pid)
    ,pidPos(pidPos)
    ,from(from)
    ,to(to)
    ,block(block)
    ,increasing(increasing){
        cerr << "Creating FOR" << endl;
    }

    void generateCode() final {
        //only loading to accumulator so no need to prepare
        if(this->from->type != Value::Type::NUM)
            this->from->prepareIfNeeded();
        if(this->to->type != Value::Type::NUM)
            this->to->prepareIfNeeded();

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

        block->generateCode();

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

        if(this->from->type != Value::Type::NUM)
            this->from->unprepareIfNeeded();
        if(this->to->type != Value::Type::NUM)
            this->to->unprepareIfNeeded();

        cerr << "End generating FOR code" << endl;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        auto iterator = memory.pushTempNamedVariable(pid, false);
        this->block->calculateVariablesUsage(numberOfNestedLoops + 1);
        memory.popTempVariable(); //iterator
    }
};

