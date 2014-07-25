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


static const char *kChefFnBegin = "chef_fn_begin";
static const char *kChefFnEnd = "chef_fn_end";
static const char *kChefBasicBlock = "chef_bb";


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
        FunctionType *FnInstrumentType = TypeBuilder<void (types::i<8>*,
                types::i<32>, types::i<32>), true>::get(M.getContext());
        FunctionType *FnEndType = TypeBuilder<void (), true>::get(M.getContext());
        FunctionType *BBInstrumentType = TypeBuilder<void (types::i<32>),
                true>::get(M.getContext());

        FnChefBegin = M.getOrInsertFunction(kChefFnBegin, FnInstrumentType);
        FnChefEnd = M.getOrInsertFunction(kChefFnEnd, FnEndType);
        FnChefBB = M.getOrInsertFunction(kChefBasicBlock, BBInstrumentType);

        for (Module::iterator I = M.begin(), IE = M.end(); I != IE; ++I) {
            Function &F = *I;

            if (!mayInstrument(F))
                continue;

            int BBCount = instrumentBasicBlocks(M, F);
            instrumentFunction(M, F, BBCount);
        }
        return true;
    }

    bool mayInstrument(Function &F) {
        if (F.empty())
            return false;

        if (&F == FnChefBegin || &F == FnChefEnd || &F == FnChefBB)
            return false;

        // XXX: Most of the S2E intrinsics are inlined.
        // We should find a more generic way of determining redundant BB
        // annotations (i.e., code regions that may never fork)
#if 0
        if (F.getName().startswith_lower("s2e_"))
            return false;

        if (F.getName().startswith_lower("__s2e"))
            return false;
#endif

        return true;
    }

    Constant *getFnNameSymbol(Module &M, Function &F) {
        Constant *FnName = ConstantDataArray::getString(F.getContext(),
                F.getName(), true);
        GlobalVariable *FnNameGVar = new GlobalVariable(M,
                FnName->getType(), true, GlobalValue::PrivateLinkage, 0);
        FnNameGVar->setInitializer(FnName);

        ConstantInt *Zero = ConstantInt::get(Type::getInt32Ty(F.getContext()), 0);
        std::vector<Constant*> GEPIndices;
        GEPIndices.push_back(Zero);
        GEPIndices.push_back(Zero);

        return ConstantExpr::getGetElementPtr(FnNameGVar, GEPIndices);
    }

    void instrumentFunction(Module &M, Function &F, int BBCount) {
        UnifyFunctionExitNodes &UFEN = getAnalysis<UnifyFunctionExitNodes>(F);

        std::vector<Value*> FnBeginArgs;
        FnBeginArgs.push_back(getFnNameSymbol(M, F));
        FnBeginArgs.push_back(ConstantInt::get(Type::getInt32Ty(F.getContext()),
                F.getName().size()));
        FnBeginArgs.push_back(ConstantInt::get(Type::getInt32Ty(F.getContext()),
                BBCount));

        CallInst::Create(FnChefBegin, FnBeginArgs, "",
                F.getEntryBlock().getFirstNonPHI());

        if (UFEN.getReturnBlock()) {
            CallInst::Create(FnChefEnd, "",
                    UFEN.getReturnBlock()->getTerminator());
        }
    }

    int instrumentBasicBlocks(Module &M, Function &F) {
        ReversePostOrderTraversal<Function*> RPOT(&F);

        int Counter = 0;
        for (ReversePostOrderTraversal<Function*>::rpo_iterator it = RPOT.begin(),
                ie = RPOT.end(); it != ie; ++it) {
            BasicBlock *bb = *it;
            CallInst::Create(FnChefBB,
                    ConstantInt::get(Type::getInt32Ty(F.getContext()), Counter),
                    "", bb->getFirstNonPHI());
            Counter++;
        }

        return Counter;
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
