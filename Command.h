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
    virtual void generateCode(stringstream& ss){

    };

    virtual cl_I getLinesCount(){
        return 0;
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

    virtual void generateCode(stringstream& ss){
        cerr << "Generating ASSIGNMENT code" << endl;
        this->expr->loadToAccumulator(ss);
        this->ident->storeFromAccumulator(ss);
        cerr << "End generating ASSIGNMENT code" << endl;
    }

    virtual cl_I getLinesCount(){
        cl_I linesCount = 0;
        linesCount += this->expr->getLoadToAccumulatorLinesCount();
        linesCount += this->ident->getStoreFromAccumulatorLinesCount();
        return linesCount;
    }
};

class Read : public Command {
public:
    Identifier* ident;

    explicit Read(Identifier* ident)
            :ident(ident){
        cerr << "Creating READ with " << ident->toString() << endl;
    }

    virtual void generateCode(stringstream& ss){
        cerr << "Generating READ code" << endl;
        GET(ss);
        ident->storeFromAccumulator(ss);
        cerr << "End generating READ code" << endl;
    }

    virtual cl_I getLinesCount(){
        cl_I linesCount = 0;
        linesCount += 1; // GET
        linesCount += ident->getStoreFromAccumulatorLinesCount();
        return linesCount;
    }
};

class Write : public Command {
public:
    Value* val;

    explicit Write(Value* val)
    :val(val){
        cerr << "Creating WRITE with " << val->toString() << endl;
    }

    virtual void generateCode(stringstream& ss){
        cerr << "Generating WRITE code" << endl;
        this->val->loadToAccumulator(ss);
        PUT(ss);
        cerr << "End generating WRITE code" << endl;
    }

    virtual cl_I getLinesCount(){
        cl_I linesCount = 0;
        linesCount += 1; // PUT
        linesCount += this->val->getLoadToAccumulatorLinesCount();
        return linesCount;
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

    virtual void generateCode(stringstream& ss) {
        cerr << "Generating IF code" << endl;
        //TODO check if num of commands > 0
        if(!this->cond->constComparision){
            cl_I jumpLength = 0;
            for(auto cmd : this->block->commands){
                jumpLength += cmd->getLinesCount();
            }

            this->cond->generateTestAndJumpIfNotSatisfied(ss, jumpLength);
            for(auto cmd : this->block->commands){
                cmd->generateCode(ss);
            }
        } else if (this->cond->constComparisionResult){
            for(auto cmd : this->block->commands){
                cmd->generateCode(ss);
            }
        }
        cerr << "End generating IF code" << endl;
    }

    virtual cl_I getLinesCount() {
        cl_I linesCount = 0;
        if(!this->cond->constComparision) {

            for (auto cmd : this->block->commands) {
                linesCount += cmd->getLinesCount();
            }

            linesCount += this->cond->getTestAndJumpIfNotSatisfiedLinesCount();
        } else if (this->cond->constComparisionResult){
            for(auto cmd : this->block->commands){
                linesCount += cmd->getLinesCount();
            }
        }

        return linesCount;
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

    virtual void generateCode(stringstream& ss) {
        cerr << "Generating IF ELSE code" << endl;
        //TODO check if num of commands > 0
        if(!this->cond->constComparision){
            cl_I jumpToElseLength = 0;
            for(auto cmd : this->block1->commands){
                jumpToElseLength += cmd->getLinesCount();
            }
            jumpToElseLength += 1; //jump outside if instruction

            this->cond->generateTestAndJumpIfNotSatisfied(ss, jumpToElseLength); //jump to else

            for(auto cmd : this->block1->commands){
                cmd->generateCode(ss);
            }

            cl_I jumpOutsideIf = machine.getLineCounter() + 1;
            for(auto cmd : this->block2->commands){
                jumpOutsideIf += cmd->getLinesCount();
            }

            JUMP(ss, jumpOutsideIf);

            for(auto cmd : this->block2->commands){
                cmd->generateCode(ss);
            }

        } else if (this->cond->constComparisionResult){
            for(auto cmd : this->block1->commands){
                cmd->generateCode(ss);
            }
        } else {
            for(auto cmd : this->block2->commands){
                cmd->generateCode(ss);
            }
        }
        cerr << "End generating IF ELSE code" << endl;
    }

    virtual cl_I getLinesCount() {
        cl_I linesCount = 0;

        if(!this->cond->constComparision) {
            linesCount += this->cond->getTestAndJumpIfNotSatisfiedLinesCount();

            for (auto cmd : this->block1->commands) {
                linesCount += cmd->getLinesCount();
            }

            linesCount += 1; //jump

            for (auto cmd : this->block2->commands) {
                linesCount += cmd->getLinesCount();
            }
        } else if (this->cond->constComparisionResult){
            for(auto cmd : this->block1->commands){
                linesCount += cmd->getLinesCount();
            }
        } else {
            for(auto cmd : this->block2->commands){
                linesCount += cmd->getLinesCount();
            }
        }

        return linesCount;
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

    virtual void generateCode(stringstream& ss) {
        cerr << "Generating WHILE code" << endl;
        //TODO check if num of commands > 0
        if(!this->cond->constComparision){

            cl_I jumpLength = 0;
            for(auto cmd : this->block->commands){
                jumpLength += cmd->getLinesCount();
            }
            jumpLength += 1; //pass last JUMP

            cl_I startPosition = machine.getLineCounter();

            this->cond->generateTestAndJumpIfNotSatisfied(ss, jumpLength);
            for(auto cmd : this->block->commands){
                cmd->generateCode(ss);
            }

            JUMP(ss, startPosition);

        } else if (this->cond->constComparisionResult) {
            cl_I startPosition = machine.getLineCounter();

            for(auto cmd : this->block->commands){
                cmd->generateCode(ss);
            }

            JUMP(ss, startPosition);
        }
        cerr << "End generating WHILE code" << endl;
    }

    virtual cl_I getLinesCount() {
        cl_I linesCount = 0;
        if(!this->cond->constComparision) {

            for (auto cmd : this->block->commands) {
                linesCount += cmd->getLinesCount();
            }

            linesCount += this->cond->getTestAndJumpIfNotSatisfiedLinesCount();

            linesCount += 1; //last JUMP
        } else if (this->cond->constComparisionResult){
            for(auto cmd : this->block->commands){
                linesCount += cmd->getLinesCount();
            }

            linesCount += 1; //last JUMP
        }

        return linesCount;
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

    virtual void generateCode(stringstream& ss){
        cerr << "Generating FOR code" << endl;
        auto iterator = memory.pushTempNamedVariable(pid, false);
        auto tmpTo = memory.pushTempVariable();

        if(iterator == nullptr){
            poserror(pidPos, "for loop temp variable redeclaration");
            return;
        }

        this->from->loadToAccumulator(ss);
        STORE(ss, iterator->memoryPtr);
        iterator->initialize();
        this->to->loadToAccumulator(ss);
        STORE(ss, tmpTo->memoryPtr);
        tmpTo->initialize();

        //first comparision - first check needed for initial condition
        //edge case - if going DOWNTO 0 cant go -1, need checking after loop if equals before iterator inc/dec
        if(this->increasing){
            LOAD(ss, tmpTo->memoryPtr);
            INC(ss);
            SUB(ss, iterator->memoryPtr);
        } else {
            LOAD(ss, iterator->memoryPtr);
            INC(ss);
            SUB(ss, tmpTo->memoryPtr);
        }

        cl_I jumpOutsideLoop = machine.getLineCounter();
        for(auto cmd : this->block->commands){
            jumpOutsideLoop += cmd->getLinesCount(); //JUMP OVER ALL BLOCKS
        }
        jumpOutsideLoop += 8; // JUMP LINE AFTER THEM, AND 7 LAST INSTRUCTIONS

        JZERO(ss, jumpOutsideLoop);

        cl_I loopStart = machine.getLineCounter();

        for(auto cmd : this->block->commands){
            cmd->generateCode(ss);
        }

        if(this->increasing){ //0 when iterator >= to
            LOAD(ss, tmpTo->memoryPtr);
            SUB(ss, iterator->memoryPtr);
        } else { // 0 when iterator <= to
            LOAD(ss, iterator->memoryPtr);
            SUB(ss, tmpTo->memoryPtr);
        }

        cl_I jumpOutsideLoop2 = machine.getLineCounter() + 5; // LINE AFTER, LOAD, INC/DEC, STORE, JUMP

        JZERO(ss, jumpOutsideLoop2);

        // update iterator
        LOAD(ss, iterator->memoryPtr);
        if(this->increasing)
            INC(ss);
        else
            DEC(ss);
        STORE(ss, iterator->memoryPtr);

        JUMP(ss, loopStart);

        memory.popTempVariable(); //iterator
        memory.popTempVariable(); //tmpTo

        cerr << "End generating FOR code" << endl;
    }

    virtual cl_I getLinesCount(){
        cl_I linesCount = 0;
        linesCount += this->from->getLoadToAccumulatorLinesCount();
        linesCount += this->to->getLoadToAccumulatorLinesCount();

        for(auto cmd : this->block->commands){
            linesCount += cmd->getLinesCount();
        }

        linesCount += 13;
        return linesCount;
    }
};

