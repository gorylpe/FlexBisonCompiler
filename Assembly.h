#pragma once

#include "Utils.h"
#include "MachineContext.h"

const int LOAD_OPS = 10;
const int STORE_OPS = 10;
const int INC_OPS = 1;
const int DEC_OPS = 1;

class AssemblyLine{
public:
    virtual void toStringstream(stringstream& ss) = 0;
};

class JumpPosition{
    unsigned long position;

public:
    void setPosition(unsigned long position){
        this->position = position;
    }

    unsigned long getPosition(){
        return position;
    }
};

class GETAssemblyLine : public AssemblyLine{
public:
    explicit GETAssemblyLine() = default;
    
    void toStringstream(stringstream& ss){
        ss << "GET"<< endl;
    }
};

class PUTAssemblyLine : public AssemblyLine{
public:
    explicit PUTAssemblyLine() = default;
    
    void toStringstream(stringstream& ss){
        ss << "PUT"<< endl;
    }
};

class SHLAssemblyLine : public AssemblyLine{
public:
    explicit SHLAssemblyLine() = default;
    
    void toStringstream(stringstream& ss){
        ss << "SHL"<< endl;
    }
};

class SHRAssemblyLine : public AssemblyLine{
public:
    explicit SHRAssemblyLine() = default;
    
    void toStringstream(stringstream& ss){
        ss << "SHR"<< endl;
    }
};

class INCAssemblyLine : public AssemblyLine{
public:
    explicit INCAssemblyLine() = default;
    
    void toStringstream(stringstream& ss){
        ss << "INC"<< endl;
    }
};

class DECAssemblyLine : public AssemblyLine{
public:
    explicit DECAssemblyLine() = default;
    
    void toStringstream(stringstream& ss){
        ss << "DEC"<< endl;
    }
};

class ZEROAssemblyLine : public AssemblyLine{
public:
    explicit ZEROAssemblyLine() = default;
    
    void toStringstream(stringstream& ss){
        ss << "ZERO"<< endl;
    }
};

class HALTAssemblyLine : public AssemblyLine{
public:
    explicit HALTAssemblyLine() = default;
    
    void toStringstream(stringstream& ss){
        ss << "HALT"<< endl;
    }
};

class JumpAssemblyLine : public AssemblyLine{
protected:
    JumpPosition* position;
public:
    explicit JumpAssemblyLine(JumpPosition* position)
    :position(position){}

    JumpPosition* getJump(){return position;}

    virtual void toStringstream(stringstream& ss) = 0;
};

class JUMPAssemblyLine : public JumpAssemblyLine{
public:
    explicit JUMPAssemblyLine(JumpPosition* position)
    :JumpAssemblyLine(position){}

    void toStringstream(stringstream& ss){
        ss << "JUMP " << position->getPosition() << endl;
    }
};

class JZEROAssemblyLine : public JumpAssemblyLine{
public:
    explicit JZEROAssemblyLine(JumpPosition* position)
    :JumpAssemblyLine(position){}

    void toStringstream(stringstream& ss){
        ss << "JZERO " << position->getPosition() << endl;
    }
};

class JODDAssemblyLine : public JumpAssemblyLine{
public:
    explicit JODDAssemblyLine(JumpPosition* position)
    :JumpAssemblyLine(position){}

    void toStringstream(stringstream& ss){
        ss << "JODD " << position->getPosition() << endl;
    }
};

class SingleMemoryPointerAssemblyLine : public AssemblyLine{
protected:
    cl_I memoryPtr;
public:
    explicit SingleMemoryPointerAssemblyLine(cl_I& memoryPtr)
    :memoryPtr(memoryPtr){}

    cl_I getMemoryPtr(){return memoryPtr;}
    
    virtual void toStringstream(stringstream& ss) = 0;
};

class LOADAssemblyLine : public SingleMemoryPointerAssemblyLine{
public:
    explicit LOADAssemblyLine(cl_I& memoryPtr)
    :SingleMemoryPointerAssemblyLine(memoryPtr){}

    void toStringstream(stringstream& ss){
        ss << "LOAD " << memoryPtr << endl;
    }
};

class LOADIAssemblyLine : public SingleMemoryPointerAssemblyLine{
public:
    explicit LOADIAssemblyLine(cl_I& memoryPtr)
    :SingleMemoryPointerAssemblyLine(memoryPtr){}

    void toStringstream(stringstream& ss){
        ss << "LOADI " << memoryPtr << endl;
    }
};

class STOREAssemblyLine : public SingleMemoryPointerAssemblyLine{
public:
    explicit STOREAssemblyLine(cl_I& memoryPtr)
    :SingleMemoryPointerAssemblyLine(memoryPtr){}

    void toStringstream(stringstream& ss){
        ss << "STORE " << memoryPtr << endl;
    }
};

class STOREIAssemblyLine : public SingleMemoryPointerAssemblyLine{
public:
    explicit STOREIAssemblyLine(cl_I& memoryPtr)
    :SingleMemoryPointerAssemblyLine(memoryPtr){}

    void toStringstream(stringstream& ss){
        ss << "STOREI " << memoryPtr << endl;
    }
};

class ADDAssemblyLine : public SingleMemoryPointerAssemblyLine{
public:
    explicit ADDAssemblyLine(cl_I& memoryPtr)
    :SingleMemoryPointerAssemblyLine(memoryPtr){}

    void toStringstream(stringstream& ss){
        ss << "ADD " << memoryPtr << endl;
    }
};

class ADDIAssemblyLine : public SingleMemoryPointerAssemblyLine{
public:
    explicit ADDIAssemblyLine(cl_I& memoryPtr)
            :SingleMemoryPointerAssemblyLine(memoryPtr){}

    void toStringstream(stringstream& ss){
        ss << "ADDI " << memoryPtr << endl;
    }
};

class SUBAssemblyLine : public SingleMemoryPointerAssemblyLine{
public:
    explicit SUBAssemblyLine(cl_I& memoryPtr)
            :SingleMemoryPointerAssemblyLine(memoryPtr){}

    void toStringstream(stringstream& ss){
        ss << "SUB " << memoryPtr << endl;
    }
};

class SUBIAssemblyLine : public SingleMemoryPointerAssemblyLine{
public:
    explicit SUBIAssemblyLine(cl_I& memoryPtr)
            :SingleMemoryPointerAssemblyLine(memoryPtr){}

    void toStringstream(stringstream& ss){
        ss << "SUBI " << memoryPtr << endl;
    }
};