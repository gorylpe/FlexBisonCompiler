#pragma once

#include "../IdentifiersSSAHelper.h"
#include "../Command.h"
#include "../IdentifiersUsagesHelper.h"

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
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Creating " << toString() << endl;
    #endif
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
        } else {
            machine.LOAD(tmpTo->memoryPtr);
            machine.INC();
            machine.SUB(iterator->memoryPtr);
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

    void simplifyExpressions() final {
        block->simplifyExpressions();
    }

    void collectUsagesData(IdentifiersUsagesHelper &helper) final {
        auto iterator = memory.pushTempNamedVariable(pid, false);

        auto ident1 = from->getIdentifier();
        if(ident1 != nullptr){
            helper.addUsage(ident1);
        }
        auto ident2 = to->getIdentifier();
        if(ident2 != nullptr){
            helper.addUsage(ident2);
        }

        block->collectUsagesData(helper);

        memory.popTempVariable(); //iterator
    }

    int searchUnusedAssignmentsAndSetForDeletion(IdentifiersUsagesHelper &helper) final {
        return block->searchUnusedAssignmentsAndSetForDeletion(helper);
    }

    int propagateValues(IdentifiersAssignmentsHelper &assgnsHelper, IdentifiersUsagesHelper &usagesHelper) final {
        return block->propagateValues(assgnsHelper, usagesHelper);
    }

    void collectAssignmentsForIdentifiers(IdentifiersAssignmentsHelper& helper) final {
        block->collectAssignmentsForIdentifiers(helper);
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

    void collectSSANumbersInIdentifiers(IdentifiersSSAHelper &stats) final {
        auto beforeForSSAs = stats.getSSAsCopy();

        IdentifiersSSAHelper & tmpStats = *stats.clone();

        auto ident1 = from->getIdentifier();
        if(ident1 != nullptr){
            tmpStats.setForUsage(ident1);
        }
        auto ident2 = to->getIdentifier();
        if(ident2 != nullptr){
            tmpStats.setForUsage(ident2);
        }
        block->collectSSANumbersInIdentifiers(tmpStats);

        auto prewhileSSAs = tmpStats.getSSAsCopy();
        /*cerr << "PRE FOR" << endl;
        cerr << tmpStats.toString() << endl;*/

        stats.mergeWithOldSSAs(prewhileSSAs);

        /*cerr << "FOR MERGED" << endl;
        cerr << stats.toString() << endl;*/

        if(ident1 != nullptr){
            stats.setForUsage(ident1);
        }
        if(ident2 != nullptr){
            stats.setForUsage(ident2);
        }

        block->collectSSANumbersInIdentifiers(stats);
       /* cerr << "AFTER FOR CALCULATIONS beforessa" << endl;
        cerr << IdentifiersSSAHelper::SSAsToString(beforeForSSAs) << endl;*/

        stats.mergeWithOldSSAs(beforeForSSAs);

        /*cerr << "AFTER FOR" << endl;
        cerr << stats.toString() << endl;*/
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
