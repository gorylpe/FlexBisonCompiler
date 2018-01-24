#pragma once

#include <vector>
#include <unordered_map>
#include "Identifier.h"

using namespace std;

class IdentifierUsages{
    bool initValue(int ssaNum){
        if(usages.find(ssaNum) == usages.end()){
            usages[ssaNum] = 0;
            return true;
        }
        return false;
    }

public:
    unordered_map<int, int> usages;

    explicit IdentifierUsages() = default;

    void increaseUsage(int ssaNum){
        initValue(ssaNum);
        usages[ssaNum]++;
    }

    int getUsage(int ssaNum){
        initValue(ssaNum);
        return usages[ssaNum];
    }

    void addUsage(set<int>& ssaNums){
        for(auto ssaNum : ssaNums){
            increaseUsage(ssaNum);
        }
    }

    string toString(const string& pid) {
        stringstream ss;
        for(int i = 0; i < usages.size(); ++i){
            ss << pid << i << " uses: " << getUsage(i) << endl;
        }
        return ss.str();
    }
};


class IdentifiersUsagesHelper {
public:
    IdentifiersUsagesHelper() = default;

    map<string, IdentifierUsages*> assignments;

    IdentifierUsages* getUsages(string pid){
        if(assignments.find(pid) == assignments.end()){
            assignments[pid] = new IdentifierUsages();
        }
        return assignments.at(pid);
    }

    void tryToAddUsageForPidpidInStore(Identifier* ident){
        if(ident->isTypePIDPID()){
            getUsages(ident->pidpid)->addUsage(ident->getSSANumsPidpid());
        }
    }

    void addUsage(Identifier *ident) {
        if(ident->isTypePID()){
            getUsages(ident->pid)->addUsage(ident->getSSANums());
        } else if(ident->isTypePIDPID()){
            getUsages(ident->pidpid)->addUsage(ident->getSSANumsPidpid());
        }
    }

    void addUsages(const vector<Identifier*>& idents) {
        for(auto ident : idents){
            addUsage(ident);
        }
    };

    static string usagesToString(map<string, IdentifierUsages*> usages) {
        stringstream ss;
        for(auto& entry : usages){
            const string& pid = entry.first;
            cerr << entry.second->toString(pid) << endl;
        }
        ss << endl;
        return ss.str();
    }

    string toString() const{
        stringstream ss;
        ss << usagesToString(assignments);
        return ss.str();
    }
};