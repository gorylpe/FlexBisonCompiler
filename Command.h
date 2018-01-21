#pragma once

#include <cln/cln.h>
#include <vector>
#include <sstream>
#include <typeinfo>
#include "Identifier.h"
#include "Expression.h"
#include "Condition.h"
#include "NumberValueStats.h"

using namespace std;
using namespace cln;

class CommandsBlock;

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

    virtual bool propagateConstants() { return false; };

    virtual void getPidVariablesBeingModified(set<Variable *> &variableSet) {}

    //used to remove not reachable ifs, unroll for's with constants
    virtual CommandsBlock *blockToReplaceWith() { return nullptr; };

    virtual void replaceCommands() {}

    //for FOR unrolling - replacing iterator with consts
    virtual void replaceValuesWithConst(string pid, cl_I number) {}


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

class Assignment : public Command {
public:
    Identifier* ident;
    Expression* expr;

    explicit Assignment(Identifier* ident, Expression* expr)
    :ident(ident)
    ,expr(expr){
        cerr << "Creating ASSIGNMENT " << toString() << endl;
    }

    Assignment(const Assignment& asg2)
    :ident(asg2.ident->clone())
    ,expr(asg2.expr->clone()){}

    Assignment* clone() const final {
        return new Assignment(*this);
    }

    void semanticAnalysis() final {
        ident->semanticAnalysisSet();
        expr->semanticAnalysis();
    }

    void generateCode() final {
        cerr << "Generating ASSIGNMENT " << toString() << endl;
        this->ident->prepareIfNeeded();
        this->expr->prepareValuesIfNeeded();

        this->expr->loadToAccumulator();
        this->ident->storeFromAccumulator();

        this->ident->unprepareIfNeeded();
        this->expr->unprepareValuesIfNeeded();
    }

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }

        cerr << toString() << endl;
    }

    string toString() final {
        return ident->toString() + " := " + expr->toString() + ";";
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

    virtual void replaceValuesWithConst(string pid, cl_I number) {
        ident->replaceValuesWithConst(pid, number);
        expr->replaceValuesWithConst(pid, number);
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        expr->collectNumberValues(stats);
    }
};

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
        cerr << "Creating " << toString() << endl;
    }

    Write(const Write& read2)
    :val(read2.val->clone()){}

    Write* clone() const final {
        return new Write(*this);
    }

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }

        cerr << toString() << endl;
    }

    string toString() final {
        return "WRITE " + val->toString() + ";";
    }

    void semanticAnalysis() final {
        val->semanticAnalysis();
    }

    void generateCode() final{
        cerr << "Generating " << toString() << endl;
        if(this->val->type != Value::Type::NUM)
            this->val->prepareIfNeeded();

        this->val->loadToAccumulator();
        machine.PUT();

        if(this->val->type != Value::Type::NUM)
            this->val->unprepareIfNeeded();
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

    virtual void replaceValuesWithConst(string pid, cl_I number) {
        val->replaceIdentifierWithConst(pid, number);
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        if(val->type == Value::Type::NUM){
            if(stats.count(val->num->num) == 0){
                stats[val->num->num] = NumberValueStats();
            }
            stats[val->num->num].addLoad(val, 1);
        }
    }
};

class If : public Command{
public:
    Condition* cond;
    CommandsBlock* block;

    If(Condition* cond, CommandsBlock* block)
    :cond(cond)
    ,block(block){
        cerr << "Creating " << toString() << endl;
    }

    If(const If& if2)
    :cond(if2.cond->clone())
    ,block(if2.block->clone()){}

    If* clone() const final {
        return new If(*this);
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

    string toString() final {
        return "IF " + cond->toString();
    }

    void semanticAnalysis() final {
        cond->semanticAnalysis();
        block->semanticAnalysis();
    }

    void generateCode() final {
        cerr << "Generating " << toString() << endl;
        this->cond->prepareValuesIfNeeded();

        auto jumpIfTrue = new JumpPosition();
        auto jumpIfFalse = new JumpPosition();

        this->cond->generateTestWithTrueFalseJumps(jumpIfTrue, jumpIfFalse);
        machine.setJumpPosition(jumpIfTrue);

        block->generateCode();

        machine.setJumpPosition(jumpIfFalse);

        this->cond->unprepareValuesIfNeeded();
        cerr << "Generating END " << toString() << endl;
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

    virtual void replaceValuesWithConst(string pid, cl_I number) {
        cond->replaceValuesWithConst(pid, number);
        block->replaceValuesWithConst(pid, number);
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        cond->collectNumberValues(stats);
        block->collectNumberValues(stats);
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
        cerr << "Creating " << toString() << endl;
    }

    IfElse(const IfElse& if2)
            :cond(if2.cond->clone())
            ,block1(if2.block1->clone())
            ,block2(if2.block2->clone()){}

    IfElse* clone() const final {
        return new IfElse(*this);
    }

    void print(int nestedLevel) final {
        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "IF " << cond->toString() << " THEN" << endl;

        block1->print(nestedLevel + 1);

        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "ELSE " << endl;

        block2->print(nestedLevel + 1);

        for(int i = 0; i < nestedLevel; ++i){
            cerr << "  ";
        }
        cerr << "ENDIF" << endl;
    }

    string toString() final {
        return "IF " + cond->toString() + " ELSE";
    }

    void semanticAnalysis() final {
        cond->semanticAnalysis();
        block1->semanticAnalysis();
        block2->semanticAnalysis();
    }

    void generateCode() final {
        cerr << "Generating " << toString() << endl;
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

        cerr << "Generating END " << toString() << endl;
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

    virtual void replaceValuesWithConst(string pid, cl_I number) {
        cond->replaceValuesWithConst(pid, number);
        block1->replaceValuesWithConst(pid, number);
        block2->replaceValuesWithConst(pid, number);
    }


    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        cond->collectNumberValues(stats);
        block1->collectNumberValues(stats);
        block2->collectNumberValues(stats);
    }
};

class While : public Command {
public:
    Condition* cond;
    CommandsBlock* block;

    While(Condition* cond, CommandsBlock* block)
    :cond(cond)
    ,block(block){
        cerr << "Creating " << toString() << endl;
    }

    While(const While& while2)
            :cond(while2.cond->clone())
            ,block(while2.block->clone()){}

    While* clone() const final {
        return new While(*this);
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

    string toString() final {
        return "WHILE " + cond->toString();
    }

    void semanticAnalysis() final {
        cond->semanticAnalysis();
        block->semanticAnalysis();
    }

    void generateCode() final {
        cerr << "Generating " << toString() << endl;
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
        cerr << "Generating END " << toString() << endl;
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

        //edge case, check if cond will be const and false at start
        if(cond->wouldConstantPropagateToConstComparision() && !cond->testConstComparisionIfPropagate()){
            //propagate constants to condition and remove this block in next optimizer loop
            cerr << "WHILE first check optimization on " << cond->toString() << endl;
            if(this->cond->propagateConstants()) {
                hasPropagated = true;
            }
        } else {
            for(auto var : variablesSet){
                cerr << "Var used inside while " << var->pid << endl;
                var->unsetConstant();
            }

            if(this->cond->propagateConstants()) {
                hasPropagated = true;
            }

            if(this->block->propagateConstants()){
                hasPropagated = true;
            }
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

    virtual void replaceValuesWithConst(string pid, cl_I number) {
        cond->replaceValuesWithConst(pid, number);
        block->replaceValuesWithConst(pid, number);
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        cond->collectNumberValues(stats);
        block->collectNumberValues(stats);
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
        cerr << "Creating " << toString() << endl;
    }

    For(const For& for2)
    :pidPos(new Position(*for2.pidPos))
    ,pid(string(for2.pid))
    ,from(for2.from->clone())
    ,to(for2.to->clone())
    ,block(for2.block->clone())
    ,increasing(for2.increasing){}

    For* clone() const final {
        return new For(*this);
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

    string toString() final {
        return "FOR " + pid + " FROM " + from->toString() + (increasing ? " TO " : " DOWNTO ") + to->toString();
    }

    void generateCode() final {
        //only loading to accumulator so no need to prepare
        this->from->prepareIfNeeded();
        this->to->prepareIfNeeded();

        cerr << "Generating " << toString() << endl;
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

        cerr << "Generating END " << toString() << endl;
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
                auto newBlock = new CommandsBlock();
                if(increasing){
                    for(cl_I i = from->num->num; i <= to->num->num; ++i){
                        CommandsBlock* currBlock = block->clone();
                        currBlock->replaceValuesWithConst(pid, i);

                        newBlock->addCommands(currBlock->commands);
                    }
                } else {
                    for(cl_I i = from->num->num; i >= to->num->num; --i){
                        CommandsBlock* currBlock = block->clone();
                        currBlock->replaceValuesWithConst(pid, i);

                        newBlock->addCommands(currBlock->commands);
                    }
                }

                cerr << "UNROLLING FOR TO " << newBlock->commands.size() << " CMDS" << endl;

                return newBlock;
            }
        }

        return nullptr;
    }

    void replaceCommands() final {
        this->block->replaceCommands();
    }

    virtual void replaceValuesWithConst(string pid, cl_I number) {
        from->replaceIdentifierWithConst(pid, number);
        to->replaceIdentifierWithConst(pid, number);
        block->replaceValuesWithConst(pid, number);
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        if(from->type == Value::Type::NUM){
            if(stats.count(from->num->num) == 0){
                stats[from->num->num] = NumberValueStats();
            }
            stats[from->num->num].addLoad(from, 1);
        }
        if(to->type == Value::Type::NUM){
            if(stats.count(to->num->num) == 0){
                stats[to->num->num] = NumberValueStats();
            }
            stats[to->num->num].addLoad(to, 1);
        }
        block->collectNumberValues(stats);
    }
};


