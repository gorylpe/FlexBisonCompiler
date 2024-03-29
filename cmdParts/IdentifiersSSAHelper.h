#pragma once

#include <set>
#include <string>
#include <vector>
#include "Identifier.h"

using namespace std;

class Assignment;
class Read;
class Write;

class IdentifierSSA{
public:
    int lastSSANum;
    int nextSSANum;
    set<int> previousSSANumsForLoad;

    explicit IdentifierSSA()
    :lastSSANum(0)
    ,nextSSANum(0)
    ,previousSSANumsForLoad(set<int>()){}

    void addStore(Identifier *ident){
        set<int> newSSANums;
        newSSANums.insert(nextSSANum);
        ident->setSSANums(newSSANums);

        previousSSANumsForLoad.clear();

        lastSSANum = nextSSANum;
        nextSSANum++;
    }

    int getLastSSANumForStore() const {
        return lastSSANum;
    }

    set<int> getLastSSANumsForLoad() const{
        int lastSSANum = getLastSSANumForStore();

        //for iterator its -1 cause it has no store
        if(lastSSANum < 0){
            return set<int>();
        }

        //ones collected in whiles ifs and so one
        set<int> lastSSANums(previousSSANumsForLoad);

        //last storing was to this one, so it will be used for load too
        lastSSANums.insert(lastSSANum);

        return lastSSANums;
    }

    //used to merge with values before loops
    void mergeWithOldIdentifierSSA(IdentifierSSA *oldSSA){
        auto oldSSANums = oldSSA->getLastSSANumsForLoad();

        previousSSANumsForLoad.insert(oldSSANums.begin(), oldSSANums.end());
    }

    //used to reset previous vals for IF ELSE without reseting nextSSANum counter
    void resetPreviousSSANumsWithOldIdentifierSSA(IdentifierSSA* oldSSA){
        lastSSANum = oldSSA->lastSSANum;
        previousSSANumsForLoad = set<int>(oldSSA->previousSSANumsForLoad);
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

class IdentifiersSSAHelper{
public:
    IdentifiersSSAHelper() = default;

    explicit IdentifiersSSAHelper(const IdentifiersSSAHelper& stats2)
    :ssa(stats2.getSSAsCopy()){}

    map<string, IdentifierSSA*> ssa;

    map<string, IdentifierSSA*> getSSAsCopy() const {
        map<string, IdentifierSSA*> ssaCopy;

        for(auto& entry : ssa){
            ssaCopy[entry.first] = new IdentifierSSA(*entry.second);
        }

        return ssaCopy;
    };

    IdentifiersSSAHelper* clone(){
        return new IdentifiersSSAHelper(*this);
    }

    void mergeWithOldSSAs(map<string, IdentifierSSA *> &oldSSAs){

        for(auto& entry : oldSSAs){
            const string& pid = entry.first;

            if(ssa.find(pid) != ssa.end()){
                IdentifierSSA* currSSA = ssa.at(pid);
                IdentifierSSA* oldSSA = entry.second;
                currSSA->mergeWithOldIdentifierSSA(oldSSA);
            }
        }
    }

    void resetToOldSSAs(map<string, IdentifierSSA *> &oldSSAs){
        for(auto& entry : oldSSAs){
            const string& pid = entry.first;

            if(ssa.find(pid) != ssa.end()){
                IdentifierSSA* currSSA = ssa.at(pid);
                IdentifierSSA* oldSSA = entry.second;
                currSSA->resetPreviousSSANumsWithOldIdentifierSSA(oldSSA);
            }
        }
    }

    IdentifierSSA* getSSA(string pid){
        if(ssa.find(pid) == ssa.end()){
            ssa[pid] = new IdentifierSSA();
        }
        return ssa.at(pid);
    }

    void setForStore(Identifier *ident){
        if(ident->isTypePID()){
            getSSA(ident->pid)->addStore(ident);
        } else if(ident->isTypePIDPID()){
            setForUsage(ident);
        }
    }

    void setForUsage(Identifier *ident) {
        if(ident->isTypePID()) {
            set<int> lastSSANums = getSSA(ident->pid)->getLastSSANumsForLoad();
            ident->setSSANums(lastSSANums);

        } else if (ident->isTypePIDPID()) {
            set<int> lastSSANumsPidpid = getSSA(ident->pidpid)->getLastSSANumsForLoad();
            ident->setSSANumsPidpid(lastSSANumsPidpid);
        }
    }

    void setForUsages(const vector<Identifier *> &idents) {
        for(auto ident : idents){
            setForUsage(ident);
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