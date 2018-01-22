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
class AssignmentsStats;

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
    virtual CommandsBlock* blockToReplaceWith() = 0;

    virtual void replaceCommands() {}

    //for FOR unrolling - replacing iterator with consts
    virtual void replaceValuesWithConst(string pid, cl_I number) {}

    //virtual AssignmentsStats collectAssignmentsStats() = 0;

    //for creating numbers optimal
    virtual void collectNumberValues(map<cl_I, NumberValueStats>& stats) {}
};

