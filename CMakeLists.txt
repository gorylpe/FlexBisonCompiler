cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 11)

project(compiler C CXX)

add_executable(compiler cmds/Command.h cmdParts/Identifier.h cmdParts/Variable.h MemoryManager.h cmdParts/Expression.cpp cmdParts/Expression.h cmdParts/Condition.h cmdParts/Value.h Program.h MachineContext.h Utils.h cmdParts/Number.h Assembly.h cmdParts/NumberValueStats.h cmds/Assignment.h cmds/Read.h cmds/Write.h cmds/If.h cmds/IfElse.h cmds/While.h cmds/For.h cmdParts/IdentifiersSSAHelper.h cmdParts/IdentifiersUsagesHelper.h cmdParts/IdentifiersAssignmentsHelper.h ProgramFlags.h ProgramFlags.h cmds/CommandsBlock.h)