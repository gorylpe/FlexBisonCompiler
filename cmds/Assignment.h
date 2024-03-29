#pragma once

#include "Command.h"
#include "../cmdParts/Expression.h"
#include "../cmdParts/IdentifiersUsagesHelper.h"

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
        #ifdef DEBUG_LOG_GENERATING_CODE
        cerr << "Generating ASSIGNMENT " << toString() << endl;
        #endif
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

    void simplifyExpressions() final {
        expr->simplifyExpression();
    }

    void collectUsagesData(IdentifiersUsagesHelper &helper) final {
        helper.tryToAddUsageForPidpidInStore(ident);
        helper.addUsages(expr->getIdentifiers());
    }

    int searchUnusedAssignmentsAndSetForDeletion(IdentifiersUsagesHelper &helper) final {
        if(ident->isTypePID() && ident->isUniquelyDefinied()){
            int usages = helper.getUsages(ident->pid)->getUsage(ident->getUniqueSSANum());
            if(usages == 0){
                this->setUnused();
                return 1;
            }
        }
        return 0;
    }

    void collectAssignmentsForIdentifiers(IdentifiersAssignmentsHelper& helper) final {
        if(ident->isUniquelyDefinied()){
            helper.addAssignment(this, ident->pid, ident->getUniqueSSANum());
        }
    }

    int propagateValues(IdentifiersAssignmentsHelper &assgnsHelper, IdentifiersUsagesHelper &usagesHelper) final {
        int propagated = 0;

        stringstream ss;

        if(pflags.verbose()){
            ss << "PROPAGATED " << toString() << "   --->   ";
        }

        if(tryToPropagatePidpid(assgnsHelper, usagesHelper, *ident)){
            if(pflags.verbose()) {
                ss << toString() << endl;
                cerr << ss.str();
            }

            propagated++;

            if(pflags.verbose()) {
                ss.clear();
                ss << "PROPAGATED " << toString() << "   --->   ";
            }
        }

        if(tryToPropagatePidpidInExpression(assgnsHelper, usagesHelper, *expr)){
            if(pflags.verbose()) {
                ss << toString() << endl;
                cerr << ss.str();
            }

            propagated++;
            expr->simplifyExpression();

            if(pflags.verbose()) {
                ss.clear();
                ss << "PROPAGATED " << toString() << "   --->   ";
            }
        }

        if(tryToPropagateExpressionsAnyTypeToSingleValue(assgnsHelper, usagesHelper, *expr)){
            if(pflags.verbose()) {
                ss << toString() << endl;
                cerr << ss.str();
            }

            propagated++;
            expr->simplifyExpression();

            if(pflags.verbose()) {
                ss.clear();
                ss << "PROPAGATED " << toString() << "   --->   ";
            }
        }

        if(tryToPropagateExpressionsValueToTwoValuesInExpression(assgnsHelper, usagesHelper, *expr)){
            if(pflags.verbose()) {
                ss << toString() << endl;
                cerr << ss.str();
            }

            propagated++;
            expr->simplifyExpression();

            if(pflags.verbose()) {
                ss.clear();
                ss << "PROPAGATED " << toString() << "   --->   ";
            }
        }

        if(tryToPropagateExpressionsAdditionToTypeAddition(assgnsHelper, usagesHelper, *expr)){
            if(pflags.verbose()) {
                ss << toString() << endl;
                cerr << ss.str();
            }

            propagated++;
            expr->simplifyExpression();

            if(pflags.verbose()) {
                ss.clear();
                ss << "PROPAGATED " << toString() << "   --->   ";
            }
        }

        if(tryToPropagateExpressionsSubtractionToTypeSubtraction(assgnsHelper, usagesHelper, *expr)){
            if(pflags.verbose()) {
                ss << toString() << endl;
                cerr << ss.str();
            }

            propagated++;
            expr->simplifyExpression();

            if(pflags.verbose()) {
                ss.clear();
                ss << "PROPAGATED " << toString() << "   --->   ";
            }
        }

        if(tryToPropagateExpressionsMultiplicationToTypeMultiplication(assgnsHelper, usagesHelper, *expr)){
            if(pflags.verbose()) {
                ss << toString() << endl;
                cerr << ss.str();
            }

            propagated++;
            expr->simplifyExpression();

            if(pflags.verbose()) {
                ss.clear();
                ss << "PROPAGATED " << toString() << "   --->   ";
            }
        }

        if(tryToPropagateExpressionsDivisionToTypeDivision(assgnsHelper, usagesHelper, *expr)){
            if(pflags.verbose()) {
                ss << toString() << endl;
                cerr << ss.str();
            }

            propagated++;
            expr->simplifyExpression();

            if(pflags.verbose()) {
                ss.clear();
                ss << "PROPAGATED " << toString() << "   --->   ";
            }
        }

        return propagated;
    }

    static bool tryToPropagateExpressionsAdditionToTypeAddition(IdentifiersAssignmentsHelper &assgnsHelper,
                                                                IdentifiersUsagesHelper &usagesHelper, Expression &expr) {
        if(expr.isTypeADDITION()){
            bool reversed;
            if(expr.val1->isTypeIDENTIFIER() && expr.val2->isTypeNUM()){
                reversed = false;
            } else if(expr.val2->isTypeIDENTIFIER() && expr.val1->isTypeNUM()) {
                reversed = true;
            } else {
                return false;
            }

            if(expr.hasPidpidOrPidnum()){
                return false;
            }

            Value* identValue = reversed ? expr.val2 : expr.val1;
            Value* numValue = reversed ? expr.val1 : expr.val2;

            auto prevExpr = getExpressionAssignedToValueWithOneUsage(assgnsHelper, usagesHelper, *identValue);
            if(prevExpr != nullptr && prevExpr->isTypeADDITION()){
                bool prevReversed;
                if(prevExpr->val1->isTypeIDENTIFIER() && prevExpr->val2->isTypeNUM()){
                    prevReversed = false;
                } else if(prevExpr->val2->isTypeIDENTIFIER() && prevExpr->val1->isTypeNUM()) {
                    prevReversed = true;
                } else {
                    //addition of 2 numbers of 2 identifiers
                    return false;
                }

                Value* prevIdentValue = prevReversed ? prevExpr->val2 : prevExpr->val1;
                Value* prevNumValue = prevReversed ? prevExpr->val1 : prevExpr->val2;

                numValue->num->num += prevNumValue->num->num;
                identValue->ident = prevIdentValue->ident->clone();

                return true;
            }
        }

        return false;
    }

    static bool tryToPropagateExpressionsMultiplicationToTypeMultiplication(IdentifiersAssignmentsHelper &assgnsHelper,
                                                                            IdentifiersUsagesHelper &usagesHelper,
                                                                            Expression &expr) {
        if(expr.isTypeMULTIPLICATION()){
            bool reversed;
            if(expr.val1->isTypeIDENTIFIER() && expr.val2->isTypeNUM()){
                reversed = false;
            } else if(expr.val2->isTypeIDENTIFIER() && expr.val1->isTypeNUM()) {
                reversed = true;
            } else {
                return false;
            }

            if(expr.hasPidpidOrPidnum()){
                return false;
            }

            Value* identValue = reversed ? expr.val2 : expr.val1;
            Value* numValue = reversed ? expr.val1 : expr.val2;

            auto prevExpr = getExpressionAssignedToValueWithOneUsage(assgnsHelper, usagesHelper, *identValue);
            if(prevExpr != nullptr && prevExpr->isTypeMULTIPLICATION()){
                bool prevReversed;
                if(prevExpr->val1->isTypeIDENTIFIER() && prevExpr->val2->isTypeNUM()){
                    prevReversed = false;
                } else if(prevExpr->val2->isTypeIDENTIFIER() && prevExpr->val1->isTypeNUM()) {
                    prevReversed = true;
                } else {
                    //addition of 2 numbers of 2 identifiers
                    return false;
                }

                Value* prevIdentValue = prevReversed ? prevExpr->val2 : prevExpr->val1;
                Value* prevNumValue = prevReversed ? prevExpr->val1 : prevExpr->val2;

                numValue->num->num *= prevNumValue->num->num;
                identValue->ident = prevIdentValue->ident->clone();

                return true;
            }
        }

        return false;
    }

    static bool tryToPropagateExpressionsDivisionToTypeDivision(IdentifiersAssignmentsHelper &assgnsHelper,
                                                                            IdentifiersUsagesHelper &usagesHelper,
                                                                            Expression &expr) {
        if(expr.isTypeDIVISION()){
            if(!(expr.val1->isTypeIDENTIFIER() && expr.val2->isTypeNUM())){
                return false;
            }

            if(expr.hasPidpidOrPidnum()){
                return false;
            }

            Value* identValue = expr.val1;
            Value* numValue = expr.val2;

            auto prevExpr = getExpressionAssignedToValueWithOneUsage(assgnsHelper, usagesHelper, *identValue);
            if(prevExpr != nullptr && prevExpr->isTypeDIVISION()){
                if(!(prevExpr->val1->isTypeIDENTIFIER() && prevExpr->val2->isTypeNUM())){
                    return false;
                }

                Value* prevIdentValue = prevExpr->val1;
                Value* prevNumValue = prevExpr->val2;

                numValue->num->num *= prevNumValue->num->num;
                identValue->ident = prevIdentValue->ident->clone();

                return true;
            }
        }

        return false;
    }

    static bool tryToPropagateExpressionsSubtractionToTypeSubtraction(IdentifiersAssignmentsHelper &assgnsHelper,
                                                                            IdentifiersUsagesHelper &usagesHelper,
                                                                            Expression &expr) {
        if(expr.isTypeSUBTRACTION()){
            if(!(expr.val1->isTypeIDENTIFIER() && expr.val2->isTypeNUM())){
                return false;
            }

            if(expr.hasPidpidOrPidnum()){
                return false;
            }

            Value* identValue = expr.val1;
            Value* numValue = expr.val2;

            auto prevExpr = getExpressionAssignedToValueWithOneUsage(assgnsHelper, usagesHelper, *identValue);
            if(prevExpr != nullptr && prevExpr->isTypeSUBTRACTION()){
                if(!(prevExpr->val1->isTypeIDENTIFIER() && prevExpr->val2->isTypeNUM())){
                    return false;
                }

                Value* prevIdentValue = prevExpr->val1;
                Value* prevNumValue = prevExpr->val2;

                numValue->num->num += prevNumValue->num->num;
                identValue->ident = prevIdentValue->ident->clone();

                return true;
            }
        }

        return false;
    }

    static bool tryToPropagateExpressionsAnyTypeToSingleValue(IdentifiersAssignmentsHelper &assgnsHelper,
                                                              IdentifiersUsagesHelper &usagesHelper, Expression &expr) {
        if(expr.isTypeVALUE()){
            auto prevExpr = getExpressionAssignedToValueWithOneUsage(assgnsHelper, usagesHelper, *expr.val1);
            if(prevExpr != nullptr){
                expr = *prevExpr->clone();
                return true;
            }
        }

        return false;
    }


    static bool tryToPropagateExpressionsValueToTwoValuesInExpression(IdentifiersAssignmentsHelper &assgnsHelper,
                                                                      IdentifiersUsagesHelper &usagesHelper,
                                                                      Expression &expr) {
        if(!expr.isTypeVALUE()) {
            bool propagated = false;

            auto prevExpr1 = getExpressionAssignedToValueWithOneUsage(assgnsHelper, usagesHelper, *expr.val1);
            if(prevExpr1 != nullptr && prevExpr1->isTypeVALUE()){
                expr.val1 = prevExpr1->val1->clone();
                propagated = true;
            }

            auto prevExpr2 = getExpressionAssignedToValueWithOneUsage(assgnsHelper, usagesHelper, *expr.val2);
            if(prevExpr2 != nullptr && prevExpr2->isTypeVALUE()){
                expr.val2 = prevExpr2->val1->clone();
                propagated = true;
            }

            return propagated;
        }

        return false;
    }

    static bool tryToPropagateExpressionsValueToTwoValuesInCondition(IdentifiersAssignmentsHelper &assgnsHelper,
                                                                      IdentifiersUsagesHelper &usagesHelper,
                                                                      Condition &cond) {
        bool propagated = false;

        auto prevExpr1 = getExpressionAssignedToValueWithOneUsage(assgnsHelper, usagesHelper, *cond.val1);
        if(prevExpr1 != nullptr && prevExpr1->isTypeVALUE()){
            cond.val1 = prevExpr1->val1->clone();
            propagated = true;
        }

        auto prevExpr2 = getExpressionAssignedToValueWithOneUsage(assgnsHelper, usagesHelper, *cond.val2);
        if(prevExpr2 != nullptr && prevExpr2->isTypeVALUE()){
            cond.val2 = prevExpr2->val1->clone();
            propagated = true;
        }

        return propagated;
    }

    static Expression* getExpressionAssignedToValueWithOneUsage(IdentifiersAssignmentsHelper &assgnsHelper,
                                                                IdentifiersUsagesHelper &usagesHelper, Value &val){
        if(!val.isTypeIDENTIFIER()) {
            return nullptr;
        }

        Identifier* ident = val.getIdentifier();
        if(!ident->isUniquelyDefinied())
            return nullptr;

        const string& pid = ident->pid;
        int ssaNum = ident->getUniqueSSANum();

        return getExpressionForPidSSANumWithOneUsageOrConst(assgnsHelper, usagesHelper, pid, ssaNum);
    }

    static Expression* getExpressionForPidSSANumWithOneUsageOrConst(IdentifiersAssignmentsHelper &assgnsHelper,
                                                                    IdentifiersUsagesHelper &usagesHelper,
                                                                    const string& pid, int ssaNum){
        if(!assgnsHelper.hasAssignment(pid, ssaNum))
            return nullptr;

        Assignment* prevAssignment = assgnsHelper.getAssignment(pid, ssaNum);
        Identifier* prevIdent = prevAssignment->ident;
        if(!prevIdent->isUniquelyDefinied())
            return nullptr;

        const string& prevPid = prevIdent->pid;
        int prevSSANum = prevIdent->getUniqueSSANum();

        int prevIdentUsages = usagesHelper.getUsages(prevPid)->getUsage(prevSSANum);

        //this 1 usage is in place to what we want to propagate
        if(prevIdentUsages == 1){
            Expression* prevExpr = prevAssignment->expr;
            if(!prevExpr->hasPidpidOrPidnum())
                return prevExpr;
            //no restrictions about propagating constants
        } else if(prevIdentUsages > 1){
            Expression* prevExpr = prevAssignment->expr;
            if(prevExpr->isTypeVALUE() && prevExpr->val1->isTypeNUM())
                return prevExpr;

        }
        return nullptr;
    }

    static bool tryToPropagatePidpidInExpression(IdentifiersAssignmentsHelper &assgnsHelper, IdentifiersUsagesHelper &usagesHelper, Expression& expr){
        bool propagated = false;
        if(expr.isTypeVALUE()){
            if(expr.val1->isTypeIDENTIFIER()){
                if(tryToPropagatePidpid(assgnsHelper, usagesHelper, *expr.val1->getIdentifier())){
                    propagated = true;
                }
            }
        } else {
            if(expr.val1->isTypeIDENTIFIER()){
                if(tryToPropagatePidpid(assgnsHelper, usagesHelper, *expr.val1->getIdentifier())){
                    propagated = true;
                }
            }
            if(expr.val2->isTypeIDENTIFIER()){
                if(tryToPropagatePidpid(assgnsHelper, usagesHelper, *expr.val2->getIdentifier())){
                    propagated = true;
                }
            }
        }
        return propagated;
    }

    static bool tryToPropagatePidpidInCondition(IdentifiersAssignmentsHelper &assgnsHelper, IdentifiersUsagesHelper &usagesHelper, Condition& cond){
        bool propagated = false;
        if(cond.val1->isTypeIDENTIFIER()){
            if(tryToPropagatePidpid(assgnsHelper, usagesHelper, *cond.val1->getIdentifier())){
                propagated = true;
            }
        }
        if(cond.val2->isTypeIDENTIFIER()){
            if(tryToPropagatePidpid(assgnsHelper, usagesHelper, *cond.val2->getIdentifier())){
                propagated = true;
            }
        }
        return propagated;
    }

    static bool tryToPropagatePidpid(IdentifiersAssignmentsHelper &assgnsHelper, IdentifiersUsagesHelper &usagesHelper, Identifier &ident){
        if(!ident.isTypePIDPID())
            return false;

        if(!ident.isUniquelyDefiniedPidpid())
            return false;

        const string& pid = ident.pidpid;
        int ssaNum = ident.getUniqueSSANumPidpid();

        auto prevExpr = getExpressionForPidSSANumWithOneUsageOrConst(assgnsHelper, usagesHelper, pid, ssaNum);
        if(prevExpr != nullptr && prevExpr->isTypeVALUE()){
            if(prevExpr->val1->isTypeNUM()){
                ident.setFromPidpidToPidnum(prevExpr->val1->num->num);
            }
        }
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

    void collectSSANumbersInIdentifiers(IdentifiersSSAHelper &stats) final {
        stats.setForUsages(expr->getIdentifiers());
        stats.setForStore(ident);
    }

    void setUnused(){
        unused = true;
    }

    void collectNumberValues(map<cl_I, NumberValueStats>& stats) final {
        expr->collectNumberValues(stats);
    }
};
