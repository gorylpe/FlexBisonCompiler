#pragma once

#include <cln/cln.h>
#include <vector>
#include <sstream>
#include <typeinfo>
#include "Identifier.h"
#include "Expression.h"
#include "Condition.h"

using namespace std;
using namespace cln;

class CommandsBlock;

class Command {
public:
    virtual void semanticAnalysis() = 0;

    virtual void generateCode() = 0;

    virtual bool equals(Command* command){
        return false;
    }

    virtual void print(int nestedLevel) = 0;

    virtual void calculateVariablesUsage(cl_I numberOfNestedLoops) = 0;

    virtual bool propagateConstants() { return false; };

    virtual void getPidVariablesBeingModified(set<Variable *>& variableSet) {}

    //used to remove not reachable ifs, unroll for's with constants
    virtual CommandsBlock* blockToReplaceWith() { return nullptr; };
    virtual void replaceCommands() {}
};

class If;
class IfElse;
class While;

class CommandsBlock : public Command {
public:
    vector<Command*> commands;

    void addCommand(Command* command){
        commands.push_back(command);
    }

    void print(int nestedLevel) final {
        for(auto cmd : commands){
            cmd->print(nestedLevel);
        }
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

    void replaceCommands() final {
        for(int i = 0; i < commands.size(); ++i) {
            CommandsBlock* block = commands[i]->blockToReplaceWith();

            if(block != nullptr){
                //commands.insert(commands.begin() + i, block->commands.begin(), block->commands.end());
                commands[i] = block;
            }

            commands[i]->replaceCommands();
        }
    };
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

    void semanticAnalysis() final {
        ident->semanticAnalysisSet();
        expr->semanticAnalysis();
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

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }

        cerr << ident->toString() << " := " << expr->toString() << ";" << endl;
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
        this->ident->calculateVariablesUsage(numberOfNestedLoops);
        this->expr->calculateVariablesUsage(numberOfNestedLoops);
    }

    bool propagateConstants() final {
        bool hasPropagated = false;

        if(ident->propagateConstantsInPidpid()){
            hasPropagated = true;
        }

        if(expr->propagateConstants()){
            hasPropagated = true;
        }

        if(expr->isResultConst()){
            ident->setConstant(expr->getConstValue());
        } else {
            ident->unsetConstant();
        }

        return hasPropagated;
    }

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        Variable* var = this->ident->getPidVariable();
        if(var->type == Variable::Type::PID)
            variableSet.insert(var);
    }
};

class Read : public Command {
public:
    Identifier* ident;

    explicit Read(Identifier* ident)
            :ident(ident){
        cerr << "Creating READ with " << ident->toString() << endl;
    }

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }

        cerr << "READ " << ident->toString() << ";" << endl;
    }

    void semanticAnalysis() final {
        ident->semanticAnalysisSet();
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

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        Variable* var = this->ident->getPidVariable();
        if(var->type == Variable::Type::PID)
            variableSet.insert(var);
    }
};

class Write : public Command {
public:
    Value* val;

    explicit Write(Value* val)
    :val(val){
        cerr << "Creating WRITE with " << val->toString() << endl;
    }

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }

        cerr << "WRITE " << val->toString() << ";" << endl;
    }

    void semanticAnalysis() final {
        val->semanticAnalysis();
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

    bool propagateConstants() final {
        bool hasPropagated = false;

        if(val->propagateConstant()){
            hasPropagated = true;
        }

        return hasPropagated;
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

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }

        cerr << "IF " << cond->toString() << " THEN" << endl;
        block->print(nestedLevel + 1);

        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "ENDIF" << endl;
    }

    void semanticAnalysis() final {
        cond->semanticAnalysis();
        block->semanticAnalysis();
    }

    void generateCode() final {
        cerr << "Generating IF code" << endl;
        this->cond->prepareValuesIfNeeded();

        auto jumpIfTrue = new JumpPosition();
        auto jumpIfFalse = new JumpPosition();

        this->cond->generateTestWithTrueFalseJumps(jumpIfTrue, jumpIfFalse);
        machine.setJumpPosition(jumpIfTrue);

        block->generateCode();

        machine.setJumpPosition(jumpIfFalse);

        this->cond->unprepareValuesIfNeeded();
        cerr << "End generating IF code" << endl;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->cond->calculateVariablesUsage(numberOfNestedLoops);
        this->block->calculateVariablesUsage(numberOfNestedLoops);
    }

    bool propagateConstants() final {
        bool hasPropagated = false;

        if(cond->propagateConstants())
            hasPropagated = true;

        if(block->propagateConstants())
            hasPropagated = true;

        return hasPropagated;
    }

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        this->block->getPidVariablesBeingModified(variableSet);
    }

    CommandsBlock* blockToReplaceWith() final{
        if(cond->isComparisionConst()){
            if(cond->getComparisionConstResult()){
                return block;
            } else {
                return new CommandsBlock();
            }
        }

        return nullptr;
    }

    void replaceCommands() final {
        this->block->replaceCommands();
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

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }

        cerr << "IF " << cond->toString() << " THEN" << endl;
        block1->print(nestedLevel + 1);
        cerr << "ELSE " << endl;
        block1->print(nestedLevel + 1);
        cerr << "ENDIF" << endl;
    }

    void semanticAnalysis() final {
        cond->semanticAnalysis();
        block1->semanticAnalysis();
        block2->semanticAnalysis();
    }

    void generateCode() final {
        cerr << "Generating IF ELSE code" << endl;
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

        cerr << "End generating IF ELSE code" << endl;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->cond->calculateVariablesUsage(numberOfNestedLoops);
        this->block1->calculateVariablesUsage(numberOfNestedLoops);
        this->block2->calculateVariablesUsage(numberOfNestedLoops);
    }

    bool propagateConstants() final {
        bool hasPropagated = false;

        if(cond->propagateConstants())
            hasPropagated = true;

        if(block1->propagateConstants())
            hasPropagated = true;

        if(block2->propagateConstants())
            hasPropagated = true;

        return hasPropagated;
    }

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        this->block1->getPidVariablesBeingModified(variableSet);
        this->block2->getPidVariablesBeingModified(variableSet);
    }

    CommandsBlock* blockToReplaceWith() final{
        if(cond->isComparisionConst()){
            if(cond->getComparisionConstResult()){
                return block1;
            } else {
                return block2;
            }
        }

        return nullptr;
    }

    void replaceCommands() final {
        this->block1->replaceCommands();
        this->block2->replaceCommands();
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

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "WHILE " << cond->toString() << " DO" << endl;

        block->print(nestedLevel + 1);

        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "ENDWHILE " << endl;
    }

    void semanticAnalysis() final {
        cond->semanticAnalysis();
        block->semanticAnalysis();
    }

    void generateCode() final {
        cerr << "Generating WHILE code" << endl;
        if(!this->cond->isComparisionConst()){
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
        } else if (this->cond->getComparisionConstResult()) {
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

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        this->block->getPidVariablesBeingModified(variableSet);
    }

    bool propagateConstants() final {
        bool hasPropagated = false;

        set<Variable*> variablesSet;

        this->block->getPidVariablesBeingModified(variablesSet);

        for(auto var : variablesSet){
            var->unsetConstant();
        }

        if(this->block->propagateConstants()){
            hasPropagated = true;
        }

        return hasPropagated;
    }


    CommandsBlock* blockToReplaceWith() final{
        if(cond->isComparisionConst()){
            if(!cond->getComparisionConstResult()){
                return new CommandsBlock();
            }
        }

        return nullptr;
    }

    void replaceCommands() final {
        this->block->replaceCommands();
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

    void semanticAnalysis() final {
        from->semanticAnalysis();
        to->semanticAnalysis();
        auto iterator = memory.pushTempNamedVariable(pid, false);
        if(iterator == nullptr){
            poserror(pidPos, "for loop temp variable redeclaration");
            return;
        }

        iterator->initialize();
        block->semanticAnalysis();
        memory.popTempVariable(); //iterator

    }

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "FOR " << pid << " FROM " << from->toString() << (increasing ? " TO " : " DOWNTO ") << to->toString() << " DO " << endl;

        block->print(nestedLevel + 1);

        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "ENDFOR" << endl;
    }

    void generateCode() final {
        //only loading to accumulator so no need to prepare
        this->from->prepareIfNeeded();
        this->to->prepareIfNeeded();

        cerr << "Generating FOR code" << endl;
        auto iterator = memory.pushTempNamedVariable(pid, false);
        auto tmpTo = memory.pushTempVariable();

        auto loopStart = new JumpPosition();
        auto loopOutside = new JumpPosition();

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

        this->from->unprepareIfNeeded();
        this->to->unprepareIfNeeded();

        cerr << "End generating FOR code" << endl;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        auto iterator = memory.pushTempNamedVariable(pid, false);
        this->block->calculateVariablesUsage(numberOfNestedLoops + 1);
        memory.popTempVariable(); //iterator
    }

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        this->block->getPidVariablesBeingModified(variableSet);
    }

    bool propagateConstants() final {
        bool hasPropagated = false;

        if(from->propagateConstant()){
            hasPropagated = true;
        }

        if(to->propagateConstant()){
            hasPropagated = true;
        }

        set<Variable*> variablesSet;

        auto iterator = memory.pushTempNamedVariable(pid, false);
        this->block->getPidVariablesBeingModified(variablesSet);

        for(auto var : variablesSet){
            var->unsetConstant();
        }

        if(this->block->propagateConstants()){
            hasPropagated = true;
        }
        memory.popTempVariable(); //iterator

        return hasPropagated;
    }

    CommandsBlock* blockToReplaceWith() final{
        if(from->type == Value::Type::NUM && to->type == Value::Type::NUM){
            if(increasing && from->num->num > to->num->num){
                return new CommandsBlock();
            } else if(!increasing && from->num->num < to->num->num){
                return new CommandsBlock();
            } else {
                auto block = new CommandsBlock();
                if(increasing){
                    for(cl_I i = from->num->num; i <= to->num->num; ++i){

                    }
                }
            }
        }

        return nullptr;
    }

    void replaceCommands() final {
        this->block->replaceCommands();
    }
};

