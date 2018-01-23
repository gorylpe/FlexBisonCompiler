#pragma once

#include "../Command.h"
#include "../Expression.h"

class Assignment : public Command {
public:
    Identifier* ident;
    Expression* expr;

    bool unused;

    explicit Assignment(Identifier* ident, Expression* expr)
    :ident(ident)
    ,expr(expr)
    ,unused(false){
    #ifdef DEBUG_LOG_CONSTRUCTORS
        cerr << "Creating ASSIGNMENT " << toString() << endl;
    #endif
    }

    Assignment(const Assignment& asg2)
    :ident(asg2.ident->clone())
    ,expr(asg2.expr->clone())
    ,unused(false){}

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
        cerr << " asg" << endl;

        return true;
    }

    void calculateVariablesUsage(cl_I numberOfNestedLoops) final {
        this->ident->calculateVariablesUsage(numberOfNestedLoops);
        this->expr->calculateVariablesUsage(numberOfNestedLoops);
    }

    void simplifyExpressions() final {
        expr->simplifyExpression();
    }

    bool propagateValues(IdentifiersSSA &stats) final {
        bool hasPropagated = false;

        /*if(!ident->isTypePID())
            return false;

        cerr << "ASSIGNMENT PROPAGATE VALUE" << endl;
        if(ident->isUniquelyDefinied()){
            const string& pid = ident->pid;
            const int SSANum = ident->getUniqueSSANum();

            int SSAUses = stats.getUse(pid)->getUsageForSSANum(SSANum);
            if(SSAUses == 0){
                setUnused();
                hasPropagated = true;
            }
        }

        if(expr->isTypeVALUE()){
            if(expr->val1->isTypeIDENTIFIER()){
                Identifier* previousIdent = expr->val1->getIdentifier();

                if(!previousIdent->isTypePID())
                    return false;

                if(previousIdent->isUniquelyDefinied()){
                    const string& prevPid = previousIdent->pid;
                    const int prevSSANum = previousIdent->getUniqueSSANum();

                    int prevSSAUses = stats.getUse(prevPid)->getUsageForSSANum(prevSSANum);
                    if(prevSSAUses == 1){
                        if(stats.getAssignments(prevPid)->hasAssignmentForSSANum(prevSSANum)){
                            Assignment* prevAssignment = stats.getAssignments(prevPid)->getAssignmentForSSANum(prevSSANum);
                            Expression* prevExpr = prevAssignment->expr;

                            cerr << "OLD EXPR" << endl;
                            cerr << toString() << endl;
                            expr->type = prevExpr->type;
                            expr->val1 = prevExpr->val1;
                            expr->val2 = prevExpr->val2;

                            cerr << "NEW EXPR" << endl;
                            cerr << toString() << endl;

                            stats.getUse(prevPid)->removeUsageForSSANum(prevSSANum);

                            hasPropagated = true;
                        }
                    }
                }
            }
        } else {
            propagateValue(expr->val1, stats);
            propagateValue(expr->val2, stats);
        }*/

        return hasPropagated;
    }

    static bool propagateValue(Value* val, IdentifiersSSA &stats) {
        bool hasPropagated = false;

        /*if(val->isTypeIDENTIFIER()){
            const string& pid = val->ident->pid;

            if(!val->ident->isTypePID()){
                return false;
            }

            if(val->ident->isUniquelyDefinied()){
                int valSSANum = val->ident->getUniqueSSANum();
                if(stats.getAssignments(pid)->hasAssignmentForSSANum(valSSANum)){
                    Assignment* assign = stats.getAssignments(pid)->getAssignmentForSSANum(valSSANum);

                    int ssaUses = stats.getUse(pid)->getUsageForSSANum(valSSANum);
                    if(ssaUses > 0){
                        if(assign->expr->isTypeVALUE()){
                            val->type = assign->expr->val1->type;
                            val->ident = assign->expr->val1->ident;
                            val->num = assign->expr->val1->num;
                            cerr << "PROPAGATING VALUE" << endl;
                            cerr << pid << " <- " << assign->toString() << endl;

                            stats.getUse(pid)->removeUsageForSSANum(valSSANum);

                            hasPropagated = true;
                        }
                    }
                }
            }
        }*/

        return hasPropagated;
    }

    void getPidVariablesBeingModified(set<Variable *>& variableSet) final {
        Variable* var = ident->getPidVariable();
        if(var->type == Variable::Type::PID)
            variableSet.insert(var);
    }

    CommandsBlock* blockToReplaceWith() final {
        if(expr->equals(ident)){
            return new CommandsBlock();
        }

        if(unused){
            return new CommandsBlock();
        }

        return nullptr;
    }

    void replaceValuesWithConst(string pid, cl_I number) final {
        ident->replaceValuesWithConst(pid, number);
        expr->replaceValuesWithConst(pid, number);
    }

    void calculateSSANumbersInIdentifiers(IdentifiersSSA &prevStats) final {
        prevStats.addUsages(expr->getIdentifiers());
        prevStats.addStore(ident);
    }

    void setUnused(){
        unused = true;
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        expr->collectNumberValues(stats);
    }
};
