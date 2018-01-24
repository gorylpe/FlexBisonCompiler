#pragma once

#include <cln/cln.h>
#include <vector>
#include <sstream>
#include <typeinfo>
#include "../cmdParts/Identifier.h"
#include "../cmdParts/Condition.h"
#include "../cmdParts/NumberValueStats.h"
#include "../cmdParts/IdentifiersUsagesHelper.h"
#include "../cmdParts/IdentifiersAssignmentsHelper.h"

using namespace std;
using namespace cln;

class CommandsBlock;
class IdentifiersSSAHelper;

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

    virtual void simplifyExpressions() = 0;

    //used to remove not reachable ifs, unroll for's with constants, remove assignments
    virtual CommandsBlock* blockToReplaceWith() = 0;

    virtual void replaceCommands() {}

    //for FOR unrolling - replacing iterator with consts
    virtual void replaceValuesWithConst(string pid, cl_I number) {}

    virtual void collectSSANumbersInIdentifiers(IdentifiersSSAHelper &helper) = 0;

    virtual void collectUsagesData(IdentifiersUsagesHelper &helper) = 0;

    virtual int searchUnusedAssignmentsAndSetForDeletion(IdentifiersUsagesHelper &helper) = 0;

    virtual void collectAssignmentsForIdentifiers(IdentifiersAssignmentsHelper& helper) = 0;

    virtual int propagateValues(IdentifiersAssignmentsHelper &assgnsHelper, IdentifiersUsagesHelper &usagesHelper) = 0;

        //for creating numbers optimal
    virtual void collectNumberValues(map<cl_I, NumberValueStats>& stats) {}
};
