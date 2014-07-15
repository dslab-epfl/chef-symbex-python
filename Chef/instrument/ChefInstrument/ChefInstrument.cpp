/*
 * Copyright 2014 EPFL. All rights reserved.
 */

#include <llvm/Pass.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/TypeBuilder.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/CFG.h>
#include <llvm/ADT/PostOrderIterator.h>

#include <llvm/PassManager.h>
#include <llvm/PassRegistry.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Utils/UnifyFunctionExitNodes.h>

using namespace llvm;


namespace {

struct ChefInstrument : public ModulePass {
    static char ID;

    ChefInstrument() :
            ModulePass(ID),
            FnChefBegin(0),
            FnChefEnd(0),
            FnChefBB(0) {

    }

    virtual void getAnalysisUsage(AnalysisUsage &AU) const {
        AU.setPreservesCFG();
        AU.addRequired<UnifyFunctionExitNodes>();
    }

    virtual bool runOnModule(Module &M) {
        FunctionType *FnInstrumentType = TypeBuilder<void (types::i<8>*),
                true>::get(M.getContext());
        FunctionType *BBInstrumentType = TypeBuilder<void (types::i<32>),
                true>::get(M.getContext());

        FnChefBegin = M.getOrInsertFunction("chef_fn_begin", FnInstrumentType);
        FnChefEnd = M.getOrInsertFunction("chef_fn_end", FnInstrumentType);
        FnChefBB = M.getOrInsertFunction("chef_bb", BBInstrumentType);

        for (Module::iterator it = M.begin(), ie = M.end(); it != ie; ++it) {
            Function &F = *it;

            if (F.empty() || &F == FnChefBegin || &F == FnChefEnd || &F == FnChefBB) {
                // errs() << "Skipping: " << F.getName() << '\n';
                continue;
            }

            // errs() << "Processing: " << F.getName() << '\n';
            instrumentBasicBlocks(M, F);
            instrumentFunction(M, F);
        }
        return true;
    }

    Constant *getFnNameSymbol(Module &M, Function &F) {
        Constant *fn_name = ConstantDataArray::getString(F.getContext(),
                F.getName(), true);
        GlobalVariable *fn_name_gvar = new GlobalVariable(M,
                fn_name->getType(), true, GlobalValue::PrivateLinkage, 0);
        fn_name_gvar->setInitializer(fn_name);

        ConstantInt *zero = ConstantInt::get(Type::getInt32Ty(F.getContext()), 0);
        std::vector<Constant*> indices;
        indices.push_back(zero);
        indices.push_back(zero);

        return ConstantExpr::getGetElementPtr(fn_name_gvar, indices);
    }

    void instrumentFunction(Module &M, Function &F) {
        Constant *fn_name_ptr = getFnNameSymbol(M, F);
        UnifyFunctionExitNodes &UFEN = getAnalysis<UnifyFunctionExitNodes>(F);

        CallInst::Create(FnChefBegin, fn_name_ptr, "",
                F.getEntryBlock().getFirstNonPHI());

        if (UFEN.getReturnBlock()) {
            CallInst::Create(FnChefEnd, fn_name_ptr, "",
                    UFEN.getReturnBlock()->getTerminator());
        }
    }

    void instrumentBasicBlocks(Module &M, Function &F) {
        ReversePostOrderTraversal<Function*> RPOT(&F);

        int Counter = 0;
        for (ReversePostOrderTraversal<Function*>::rpo_iterator it = RPOT.begin(),
                ie = RPOT.end(); it != ie; ++it) {
            BasicBlock *bb = *it;
            CallInst::Create(FnChefBB, ConstantInt::get(Type::getInt32Ty(F.getContext()), Counter),
                    "", bb->getFirstNonPHI());
            Counter++;
        }
    }

private:
    Constant *FnChefBegin;
    Constant *FnChefEnd;
    Constant *FnChefBB;
};

}

char ChefInstrument::ID = 0;

static RegisterPass<ChefInstrument> X("chef-instrument",
        "Chef Instrumentation Pass", false, false);


static void loadChefPass(const PassManagerBuilder &Builder,
        PassManagerBase &PM) {
    PM.add(createUnifyFunctionExitNodesPass());
    PM.add(new ChefInstrument());
}


static RegisterStandardPasses RegisterChefPass(
        PassManagerBuilder::EP_OptimizerLast, loadChefPass);
