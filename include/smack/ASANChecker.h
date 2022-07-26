//
// This file is distributed under the MIT License. See LICENSE for details.
//
#ifndef ASANCHECKER_H
#define ASANCHECKER_H

#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include <map>

namespace smack {

class ASANChecker : public llvm::FunctionPass,
                            public llvm::InstVisitor<ASANChecker> {
private:
  llvm::Function *getASANCheckFunction(llvm::Module &M);

  void copyDbgMetadata(llvm::Instruction *src, llvm::Instruction *dst);
  void insertASANCheck(llvm::Value *addr, llvm::Value *size,
                               llvm::Instruction *I);

public:
  static char ID; // Pass identification, replacement for typeid
  ASANChecker() : llvm::FunctionPass(ID) {}
  virtual bool runOnFunction(llvm::Function &F) override;

  void visitReturnInst(llvm::ReturnInst &I);
  void visitLoadInst(llvm::LoadInst &I);
  void visitStoreInst(llvm::StoreInst &I);
  void visitMemSetInst(llvm::MemSetInst &I);
  void visitMemTransferInst(llvm::MemTransferInst &I);
};
} // namespace smack

#endif // ASANCHECKER_H
