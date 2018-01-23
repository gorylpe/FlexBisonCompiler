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
    int counter;
    vector<Assignment*> assignments;
    vector<int> uses;

    explicit IdentifierStats(string pid)
            :pid(pid)
            ,counter(0){}

    void addStore(Identifier *ident){
        ident->setSSACounter(counter);
        uses.push_back(0);
        counter++;
    }

    void addAssignment(Assignment* assignment) {
        assignments.push_back(assignment);
    }

    void addUsage(Identifier *ident){
        int lastCounter = counter - 1;
        //iterators has no assignments so lastCounter will be -1 always
        if(lastCounter >= 0){
            ident->setSSACounter(lastCounter);
            uses[lastCounter]++;
        }
    }

    //for pidpid, they arent identifiers and add additional usage to pids in loops
    void addUsage(){
        int lastCounter = counter - 1;
        //iterators has no assignments so lastCounter will be -1 always
        if(lastCounter >= 0){
            uses[lastCounter]++;
        }
    }

    string toString() const {
        stringstream ss;
        cerr << pid << "  counter: " << counter << endl;
        for(int i = 0; i < counter; ++i){
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

    void addStore(Identifier *ident) {
        getStats(ident->pid)->addStore(ident);
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

        getStats(ident->pid)->addStore(ident);
        getStats(ident->pid)->addAssignment(assignment);

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