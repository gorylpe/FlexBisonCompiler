#pragma once

#include <set>
#include <string>
#include <vector>
#include "../Identifier.h"
#include "../Expression.h"

using namespace std;

class Assignment;
class Read;
class Write;

class IdentifierStats{
public:
    string pid;
    int SSANum;
    vector<Assignment*> assignments;
    vector<int> uses;

    explicit IdentifierStats(string pid)
            :pid(pid)
            ,SSANum(0){}

    void addStore(Assignment* assignment, Identifier *ident){
        ident->setSSACounter(SSANum);

        assignments.push_back(assignment);
        uses.push_back(0);

        SSANum++;
    }

    bool hasAssignementToSSANum(int num) const{
        return assignments[num] != nullptr;
    }

    Assignment* getAssignementForSSANum(int num) const{
        return assignments[num];
    }

    int getUsesForSSANum(int num) const{
        return uses[num];
    }

    int getLastSSANum() const{
        return SSANum - 1;
    }

    void addUsage(Identifier *ident){
        int lastSSANum = getLastSSANum();

        //iterators has no assignments so lastSSANum will be -1 always
        if(lastSSANum >= 0){
            ident->setSSACounter(lastSSANum);
            uses[lastSSANum]++;
        }
    }

    //for pidpid, they arent identifiers and add additional usage to pids in loops
    void addUsage(){
        int lastCounter = SSANum - 1;
        //iterators has no assignments so lastCounter will be -1 always
        if(lastCounter >= 0){
            uses[lastCounter]++;
        }
    }

    string toString() const {
        stringstream ss;
        cerr << pid << "  SSANum: " << SSANum << endl;
        for(int i = 0; i < SSANum; ++i){
            ss << pid << i << "   " << uses[i] << endl;
        }
        return ss.str();
    }
};

class AssignmentsStats{
public:
    map<string, IdentifierStats*> stats;

    IdentifierStats* getStats(string pid){
        if(stats.find(pid) == stats.end()){
            stats[pid] = new IdentifierStats(pid);
        }
        return stats.at(pid);
    }

    void addRead(Identifier *ident) {
        getStats(ident->pid)->addStore(nullptr, ident);
    }

    void addUsage(Identifier *ident) {
        getStats(ident->pid)->addUsage(ident);
    }

    void addCondition(Condition* cond) {
        const vector<Identifier*>& idents = cond->getIdentifiers();
        for(auto id : idents){
            getStats(id->pid)->addUsage(id);
        }
    };

    void addExpression(Expression* expr) {
        //set last counters
        const vector<Identifier*>& idents = expr->getIdentifiers();
        for(auto id : idents){
            getStats(id->pid)->addUsage(id);
            if(id->type == Identifier::Type::PIDPID){
                getStats(id->pidpid)->addUsage();
            }
        }
    };

    void addAssignment(Assignment* assignment, Identifier* ident, Expression* expr) {
        addExpression(expr);

        getStats(ident->pid)->addStore(assignment, ident);

        if(ident->type == Identifier::Type::PIDPID){
            getStats(ident->pidpid)->addUsage();
        }
    };

    void addPidsUsedInLoops(set<string> &pids){
        for(auto pid : pids){
            getStats(pid)->addUsage();
        }
    }

    string toString() const{
        stringstream ss;
        for(auto& entry : stats){
            IdentifierStats* identStats = entry.second;
            ss << identStats->toString();
        }
        return ss.str();
    }
};