#ifndef SYSYC_TailRecurtion_HPP
#define SYSYC_TailRecurtion_HPP

#include "Module.h"
#include "Function.h"
#include "IRBuilder.h"
#include "BasicBlock.h"
#include "Instruction.h"
#include "PassManager.hpp"

class TailRecurtion : public Pass
{
private:
    Function *func_;

public:
    TailRecurtion(Module *m) : Pass(m){}
    ~TailRecurtion(){};
    void run() override;
};

#endif