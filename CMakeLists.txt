cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 11)

project(compiler C CXX)

add_executable(compiler Command.cpp Command.h Identifier.cpp Identifier.h Variable.cpp Variable.h MemoryManager.cpp MemoryManager.h Expression.cpp Expression.h Condition.cpp Condition.h Value.cpp Value.h Program.cpp Program.h MachineContext.cpp MachineContext.h Utils.h Number.cpp Number.h Assembly.h NumberValueStats.h cmds/Assignment.h cmds/Read.h cmds/Write.h cmds/If.h cmds/IfElse.h cmds/While.h cmds/For.h IdentifiersSSAHelper.h IdentifiersUsagesHelper.h IdentifiersAssignmentsHelper.h)