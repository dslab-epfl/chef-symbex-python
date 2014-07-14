/*
 * Copyright 2014 EPFL. All rights reserved.
 */

#include <llvm/Pass.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>

#include <llvm/PassManager.h>
#include <llvm/PassRegistry.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

using namespace llvm;


namespace {

struct ChefInstrument : public FunctionPass {
    static char ID;

    ChefInstrument() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
        errs() << "Hello: ";
        errs().write_escaped(F.getName()) << '\n';
        return false;
    }
};

}

char ChefInstrument::ID = 0;

static RegisterPass<ChefInstrument> X("chef-instrument",
        "Chef Instrumentation Pass", false, false);


static void loadChefPass(const PassManagerBuilder &Builder,
        PassManagerBase &PM) {
    PM.add(new ChefInstrument());
}


static RegisterStandardPasses RegisterChefPass(
        PassManagerBuilder::EP_OptimizerLast, loadChefPass);
