#pragma once

#include <sstream>
#include "Command.h"
#include "cmds/Assignment.h"

class Program {
public:
    CommandsBlock* block;

    void setBlock(CommandsBlock* block){
        this->block = block;
    }

    void ASTOptimizations(){
        block->replaceCommands();
        //block->print(0);
        block->semanticAnalysis(); //TODO REMOVE - only checking if optimizations didnt mess up
        constPropagation();
        block->print(0);
        removeUnusedAssignments();
        block->print(0);
        //TODO do in loop with const propagation
        removeUnusedAssignments();
        //additional const propagation will run Expressions optimization too if remove unused assignements left them in not optimized form
        constPropagation();
        optimizeNumbers();
    }

    //TODO DO THIS IN SMALL ARRAYS
    void constPropagation(){
        cerr << "---CONST PROPAGATION AND REPLACING COMMANDS OPTIMIZATION---" << endl;
        bool hadPropagation;
        int numOfPropagations;
        do{
            numOfPropagations = 0;
            do{
                hadPropagation = block->propagateConstants();
                //block->print(0);
                block->semanticAnalysis(); //TODO REMOVE - only checking if optimizations didnt mess up
                if(hadPropagation){
                    numOfPropagations++;
                }
            }while(hadPropagation);

            cerr << "Done " << numOfPropagations << " constants propagation loops" << endl;

            if(numOfPropagations > 0){
                cerr << "Replacing unused commands or optimize using constants" << endl;
                block->replaceCommands();
                //block->print(0);
                block->semanticAnalysis(); //TODO REMOVE - only checking if optimizations didnt mess up
            }
        }while(numOfPropagations > 0);
        cerr << "---CONST PROPAGATION AND REPLACING COMMANDS OPTIMIZATION END---" << endl << endl;
    }

    void removeUnusedAssignments(){
        AssignmentsStats stats = AssignmentsStats();
        cerr << "!!!STATS!!!" << endl;
        block->collectAssignmentsStats(stats);
        cerr << stats.toString() << endl;

        for(auto& entry : stats.stats){
            const string& pid = entry.first;
            removeUnusedAssignmentsForPid(pid, stats);
        }

        block->replaceCommands();
    }

    void removeUnusedAssignmentsForPid(const string &pid, AssignmentsStats &stats){
        IdentifierStats* pidStats = stats.getStats(pid);

        cerr << "Removing unused assignments for " << pid << endl;
        int removedNum = 0;

        for(int SSANum = 0; SSANum <= pidStats->getLastSSANum(); ++SSANum){
            //no assignement == READ to identifier
            if(!pidStats->hasAssignementToSSANum(SSANum)){
                continue;
            }

            Assignment* assign = pidStats->getAssignementForSSANum(SSANum);
            Identifier* ident = assign->ident;

            //dont remove arrays
            if(!ident->isTypePID())
                break;

            bool removed;
            removed = tryToRemoveUnusedAssignment(pidStats, SSANum);

            if(!removed){
                tryToMergeNumberOperationsInWithPreviousAssignment(stats, pidStats, SSANum);
            }

            if(removed){
                ++removedNum;
            }
        }

        cerr << "Removed " << removedNum << " assignments" << endl;
    }

    bool tryToRemoveUnusedAssignment(IdentifierStats* pidStats, int SSANum){
        Assignment* assign = pidStats->getAssignementForSSANum(SSANum);

        if(pidStats->getUsesForSSANum(assign->ident->getSSANum()) == 0){
            assign->setUnused();
        }
    }

    bool tryToMergeNumberOperationsInWithPreviousAssignment(AssignmentsStats& stats, IdentifierStats *pidStats, int SSANum){
        Assignment* assign = pidStats->getAssignementForSSANum(SSANum);
        Expression* expr = assign->expr;

        //todo can do for 2 times multiplying
        if(expr->type == Expression::Type::ADDITION){
            return tryToMergeAdditionWithPreviousAssignment(stats, expr);
        } else if(expr->type == Expression::Type::VALUE) {
            //TODO
            /* optimize this
             *  a := e;
                e := a;
                a := e;
             */
        }
        //CANT DO FOR SUBTRACTION, CAUSE IT CAN LEAD TO CUT NUMBER BECAUSE THEY CANT BE NEGATIVE
        return false;
    }

    bool tryToMergeAdditionWithPreviousAssignment(AssignmentsStats& stats, Expression *expr) {
        //addition is commutative
        bool reversed = false;

        Identifier* exprIdent = expr->val1->getIdentifier();
        Number* exprNum = expr->val2->getNumber();

        if(exprIdent == nullptr || exprNum == nullptr){
            exprIdent = expr->val2->getIdentifier();
            exprNum = expr->val1->getNumber();
            reversed = true;
        }
        if(exprIdent == nullptr)
            return false;

        IdentifierStats* previousPidStats = stats.getStats(exprIdent->pid);
        int previousPidSSANUM = exprIdent->getSSANum();

        //if previous identifier has only only usage - in current assignment
        if(previousPidStats->getUsesForSSANum(previousPidSSANUM) == 1) {
            if (stats.getStats(exprIdent->pid)->hasAssignementToSSANum(exprIdent->getSSANum())) {

                Assignment *previousAssignment = stats.getStats(exprIdent->pid)->getAssignementForSSANum(exprIdent->getSSANum());
                Expression *previousExpr = previousAssignment->expr;

                //CANT DO FOR SUBTRACTION, CAUSE IT CAN LEAD TO CUT NUMBER BECAUSE THEY CANT BE NEGATIVE
                if (previousExpr->type == Expression::Type::ADDITION) {
                    //cant do addition with ident and no number
                    if(exprNum == nullptr){
                        return false;
                    }
                    //addition is commutative
                    bool previousReversed = false;

                    Identifier *previousIdent = previousExpr->val1->getIdentifier();
                    Number *previousNum = previousExpr->val2->getNumber();

                    if(previousIdent == nullptr || previousNum == nullptr){
                        previousIdent = previousExpr->val2->getIdentifier();
                        previousNum = previousExpr->val1->getNumber();
                        previousReversed = true;
                    }
                    if(previousIdent == nullptr || previousNum == nullptr)
                        return false;

                    cerr << "MERGING " << expr->toString() << " : " << exprIdent->toString() << " -> " << previousExpr->toString() << endl;

                    (reversed ? expr->val2 : expr->val1) = previousReversed ? previousExpr->val2 : previousExpr->val1;
                    exprNum->num += previousNum->num;

                    cerr << "MERGED " << expr->toString() << endl << endl;

                    previousAssignment->setUnused();

                    return true;
                } else if (previousExpr->type == Expression::Type::VALUE) {
                    Identifier *previousIdent = previousExpr->val1->getIdentifier();

                    //TODO CAN DO HERE CONST PROPAGATION
                    if (previousIdent == nullptr) {
                        return false;
                    }

                    cerr << "MERGING " << expr->toString() << " : " << exprIdent->toString() << " -> " << previousExpr->toString() << endl;

                    (reversed ? expr->val2 : expr->val1) = previousExpr->val1;
                    cerr << expr->toString() << endl;

                    cerr << "MERGED " << expr->toString() << endl << endl;

                    previousAssignment->setUnused();

                    return true;
                }
            }
        }
        return false;
    }

    bool tryToMergeValueWithPreviousAssignment(AssignmentsStats& stats, Expression *expr){
        //addition is commutative
        bool reversed = false;

        Identifier* exprIdent = expr->val1->getIdentifier();
        Number* exprNum = expr->val2->getNumber();

        if(exprIdent == nullptr || exprNum == nullptr){
            exprIdent = expr->val2->getIdentifier();
            exprNum = expr->val1->getNumber();
            reversed = true;
        }
        if(exprIdent == nullptr)
            return false;

        IdentifierStats* previousPidStats = stats.getStats(exprIdent->pid);
        int previousPidSSANUM = exprIdent->getSSANum();

        //if previous identifier has only only usage - in current assignment
        if(previousPidStats->getUsesForSSANum(previousPidSSANUM) == 1) {
            if (stats.getStats(exprIdent->pid)->hasAssignementToSSANum(exprIdent->getSSANum())) {

                Assignment *previousAssignment = stats.getStats(exprIdent->pid)->getAssignementForSSANum(exprIdent->getSSANum());
                Expression *previousExpr = previousAssignment->expr;

                //CANT DO FOR SUBTRACTION, CAUSE IT CAN LEAD TO CUT NUMBER BECAUSE THEY CANT BE NEGATIVE
                if (previousExpr->type == Expression::Type::ADDITION) {
                    //cant do addition with ident and no number
                    if(exprNum == nullptr){
                        return false;
                    }
                    //addition is commutative
                    bool previousReversed = false;

                    Identifier *previousIdent = previousExpr->val1->getIdentifier();
                    Number *previousNum = previousExpr->val2->getNumber();

                    if(previousIdent == nullptr || previousNum == nullptr){
                        previousIdent = previousExpr->val2->getIdentifier();
                        previousNum = previousExpr->val1->getNumber();
                        previousReversed = true;
                    }
                    if(previousIdent == nullptr || previousNum == nullptr)
                        return false;

                    reversed ? expr->val2 : expr->val1 = previousReversed ? previousExpr->val2 : previousExpr->val1;
                    exprNum->num += previousNum->num;
                    previousAssignment->setUnused();

                    return true;
                } else if (previousExpr->type == Expression::Type::VALUE) {
                    Identifier *previousIdent = previousExpr->val1->getIdentifier();

                    //TODO CAN DO HERE CONST PROPAGATION
                    if (previousIdent == nullptr) {
                        return false;
                    }

                    (reversed ? expr->val2 : expr->val1) = previousExpr->val1;
                    cerr << expr->toString() << endl;

                    previousAssignment->setUnused();

                    return true;
                }
            }
        }
        return false;
    }


    void optimizeNumbers(){
        map<cl_I, NumberValueStats> stats;

        block->collectNumberValues(stats);

        for(auto& entry : stats){
            const cl_I& num = entry.first;
            const auto& stat = entry.second;

            cerr << "---NUMBER " << num << " ---" << endl;
            cerr << stat.toString() << endl;
        }

        for(auto& entry : stats){
            const cl_I& num = entry.first;
            const auto& stat = entry.second;

            //if loading saved number is not profitable
            if(num <= LOAD_OPS){
                continue;
            }

            cerr << "Optimizing " << num << endl;

            const cl_I opsToLoadSingle = Number::getLoadToAccumulatorOperations(num);

            const cl_I& opsToAddSub = num * (stat.numberOfAdditions + stat.numberOfSubtractions);
            const cl_I& opsToAddSubStored = opsToLoadSingle + STORE_OPS + LOAD_OPS * (stat.numberOfAdditions + stat.numberOfSubtractions);

            cerr << "Load ops              " << opsToLoadSingle << endl;

            cerr << "Ops to add+sub stored " << opsToAddSubStored << endl;
            cerr << "Ops to add+sub        " << opsToAddSub << endl;

            bool profitable = false;

            //can do both at once cause same ops to inc/dec (1)
            if(opsToAddSubStored < opsToAddSub){
                profitable = true;
            }

            if(profitable){
                cerr << "Optimizing subtractions and additions" << endl;
                string pid = memory.addNumberVariable();

                auto number = new Number(nullptr, num);
                number->loadToAccumulator();
                delete number;

                Identifier ident(new Position(0,0,0,0), pid);
                ident.storeFromAccumulator();

                for(auto valSub : stat.valsSubtracted){
                    valSub->setIdentifier(new Identifier(ident));
                }

                for(auto valAdd : stat.valsAdded){
                    valAdd->setIdentifier(new Identifier(ident));
                }

                if(LOAD_OPS < opsToLoadSingle){
                    cerr << "Optimizing loads" << endl;
                    for(auto valLoad : stat.valsLoaded){
                        valLoad->setIdentifier(new Identifier(ident));
                    }
                }
            }
        }

    }

    string generateCode(){
        block->semanticAnalysis();

        this->ASTOptimizations();

        cerr << "---OPTIMIZED CODE---" << endl;
        block->print(0);
        cerr << "---END OPTIMIZED CODE---" << endl << endl;

        //after removing unused variables

        this->block->calculateVariablesUsage(0);

        memory.optimize();

        cerr << "---GENERATING ASSEMBLY CODE---" << endl;
        this->block->generateCode();
        machine.HALT();
        cerr << "---GENERATING ASSEMBLY CODE END---" << endl << endl;

        machine.optimize();

        cerr << "---SAVING ASSEMBLY CODE---" << endl;
        stringstream ss;
        machine.generateCode(ss);

        return ss.str();
    }
};
