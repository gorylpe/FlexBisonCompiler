#pragma once

#include <set>
#include <string>
#include <vector>
#include "Identifier.h"

using namespace std;

class Assignment;
class Read;
class Write;

class IdentifierAssignment{
public:
    vector<Assignment*> assignments;

    explicit IdentifierAssignment()
    :assignments(vector<Assignment*>()){}

    void addAssignment(Assignment* assignment){
        assignments.push_back(assignment);
    }

    bool hasAssignmentForSSANum(int num){
        return assignments[num] != nullptr;
    }

    Assignment* getAssignmentForSSANum(int num){
        return assignments[num];
    }
};

class IdentifierSSA{
public:
    int nextSSANum;
    set<int> previousSSANumsForLoad;

    explicit IdentifierSSA()
    :nextSSANum(0)
    ,previousSSANumsForLoad(set<int>()){}

    void addStore(Identifier *ident){
        set<int> newSSANums;
        newSSANums.insert(nextSSANum);
        ident->setSSANums(newSSANums);

        previousSSANumsForLoad.clear();

        nextSSANum++;
    }

    int getLastSSANumForStore() const {
        return nextSSANum - 1;
    }

    set<int> getLastSSANumsForLoad() const{
        //ones collected in whiles ifs and so one
        set<int> lastSSANums(previousSSANumsForLoad);

        //last storing was to this one, so logically it will be used for load too
        lastSSANums.insert(getLastSSANumForStore());

        return lastSSANums;
    }

    //used to merge with values before loops
    void mergeWithNewIdentifierSSA(IdentifierSSA *oldSSA){
        auto oldSSANums = oldSSA->getLastSSANumsForLoad();

        previousSSANumsForLoad.insert(oldSSANums.begin(), oldSSANums.end());
    }

    string toString(const string& pid) const {
        auto lastSSANums = getLastSSANumsForLoad();
        stringstream ss;

        ss << "Last assign:  " << pid << "\033[1;32m" << getLastSSANumForStore() << "\033[0m" << endl;
        ss << "Last to load: " << pid;
        ss << "\033[1;32m";
        if(previousSSANumsForLoad.size() == 1){
            ss << *previousSSANumsForLoad.begin();
        } else {
            ss << "(";
            bool first = true;
            for(const auto& ssa : lastSSANums){
                if(!first)
                    ss << ", ";
                first = false;
                ss << ssa;
            }
            ss << ")";
        }
        ss << "\033[0m" << endl;

        return ss.str();
    }
};

class IdentifierUses{
public:
    vector<int> uses;

    explicit IdentifierUses()
    :uses(vector<int>()){}

    IdentifierUses(const IdentifierUses& oldIdentifierUses)
    :uses(oldIdentifierUses.uses){}

    void addStore(){
        uses.push_back(0);
    }

    void addUsagea(Identifier *ident, set<int>& lastSSANums){
        //iterators has no assignments so lastSSANums would have -1 inside
        if(lastSSANums.find(-1) == lastSSANums.end()){
            ident->setSSANums(lastSSANums);
            for(auto lastSSANum : lastSSANums){
                uses[lastSSANum]++;
            }
        }
    }

    //for pidpid, they arent identifiers
    void addUsagea(set<int>& lastSSANums){
        //iterators has no assignments so lastSSANums would have -1 inside
        if(lastSSANums.find(-1) == lastSSANums.end()){
            for(auto lastSSANum : lastSSANums){
                uses[lastSSANum]++;
            }
        }
    }

    int getUsageForSSANum(int num){
        return uses[num];
    }

    int removeUsageForSSANum(int num){
        uses[num]--;
    }

    string toString(const string& pid) const {
        stringstream ss;
        for(int i = 0; i < uses.size(); ++i){
            ss << pid << i << " uses: " << uses[i] << endl;
        }
        return ss.str();
    }
};

class IdentifiersSSA{
public:
    IdentifiersSSA() = default;

    explicit IdentifiersSSA(const IdentifiersSSA& stats2)
    :ssa(stats2.getSSAsCopy()){}

    map<string, IdentifierSSA*> ssa;

    map<string, IdentifierSSA*> getSSAsCopy() const {
        map<string, IdentifierSSA*> ssaCopy;

        for(auto& entry : ssa){
            ssaCopy[entry.first] = new IdentifierSSA(*entry.second);
        }

        return ssaCopy;
    };

    IdentifiersSSA* clone(){
        return new IdentifiersSSA(*this);
    }

    void mergeWithSSAs(map<string, IdentifierSSA *> &newSSAs){

        for(auto& entry : newSSAs){
            const string& pid = entry.first;

            if(ssa.find(pid) != ssa.end()){
                IdentifierSSA* currSSA = ssa.at(pid);
                IdentifierSSA* oldSSA = entry.second;
                currSSA->mergeWithNewIdentifierSSA(oldSSA);
            }
        }
    }

    IdentifierSSA* getSSA(string pid){
        if(ssa.find(pid) == ssa.end()){
            ssa[pid] = new IdentifierSSA();
        }
        return ssa.at(pid);
    }

    void addStore(Identifier* ident){
        getSSA(ident->pid)->addStore(ident);
        cerr << "STORE " << ident->toString() << endl;
    }

    void addUsage(Identifier *ident) {
        set<int> lastSSANums = getSSA(ident->pid)->getLastSSANumsForLoad();

        ident->setSSANums(lastSSANums);
    }

    void addUsages(const vector<Identifier*>& idents) {
        for(auto ident : idents){
            set<int> lastSSANums = getSSA(ident->pid)->getLastSSANumsForLoad();

            ident->setSSANums(lastSSANums);
        }
    };

    static string SSAsToString(map<string, IdentifierSSA*> ssa) {
        stringstream ss;
        for(auto& entry : ssa){
            const string& pid = entry.first;
            cerr << entry.second->toString(pid) << endl;
        }
        ss << endl;
        return ss.str();
    }

    string toString() const{
        stringstream ss;
        ss << SSAsToString(ssa);
        return ss.str();
    }
};