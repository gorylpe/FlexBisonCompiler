#pragma once
#include <iostream>
#include <string>
#include "Position.h"

using namespace std;

#define memory MachineContext::getInstance().mem
#define machine MachineContext::getInstance()

//#define DEBUG_LOG_CONSTRUCTORS

void yyerror(const char*);
void poserror(Position* pos, const string& s);
