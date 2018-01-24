#pragma once

#include <vector>
#include "Identifier.h"

using namespace std;

class Usages{
    bool initUsage(int ssaNum){
        if(usage.find(ssaNum) == usage.end()){
            usage[ssaNum] = 0;
            return true;
        }
        return false;
    }

public:
    map<int, int> usage;

    explicit Usages() = default;

    void increaseUsage(int ssaNum){
        initUsage(ssaNum);
        usage[ssaNum]++;
    }

    void decreaseUsage(int ssaNum){
        if(!initUsage(ssaNum)){
            usage[ssaNum]--;
        }
    }

    int getUsage(int ssaNum){
        initUsage(ssaNum);
        return usage[ssaNum];
    }

    void addUsage(set<int>& ssaNums){
        for(auto ssaNum : ssaNums){
            increaseUsage(ssaNum);
        }
    }

    string toString(const string& pid) {
        stringstream ss;
        for(int i = 0; i < usage.size(); ++i){
            ss << pid << i << " uses: " << getUsage(i) << endl;
        }
        return ss.str();
    }
};


class IdentifiersUsagesHelper {
public:
    IdentifiersUsagesHelper() = default;

    map<string, Usages*> usages;

    Usages* getUsages(string pid){
        if(usages.find(pid) == usages.end()){
            usages[pid] = new Usages();
        }
        return usages.at(pid);
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

    static string usagesToString(map<string, Usages*> usages) {
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
        ss << usagesToString(usages);
        return ss.str();
    }
};