//
// This file is distributed under the MIT License. See LICENSE for details.
//
#include "smack/ASANChecker.h"
#include "smack/Debug.h"
#include "smack/Naming.h"
#include "smack/SmackOptions.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueSymbolTable.h"

namespace smack {

using namespace llvm;

Function *ASANChecker::getASANCheckFunction(Module &M) {
  auto F = M.getFunction("__SMACK_check_asan");
  //auto F = M.getFunction("assert");
  //auto F = M.getFunction("__SMACK_new_ASAN");

  assert(F && "Asan check function must be present.");
  F->setDoesNotAccessMemory();
  F->setDoesNotThrow();
  return F;
}

void ASANChecker::copyDbgMetadata(Instruction *src, Instruction *dst) {
  dst->setMetadata("dbg", src->getMetadata("dbg"));
}

void ASANChecker::insertASANCheck(Value *addr, Value *size,
                                                  Instruction *I) {
  auto &M = *I->getParent()->getParent()->getParent();
  auto &C = M.getContext();
  auto T = PointerType::getUnqual(Type::getInt8Ty(C));
  auto ptrArg = CastInst::Create(Instruction::BitCast, addr, T, "", I);
  auto sizeArg = CastInst::CreateBitOrPointerCast(size, T, "", I);
  auto ci =
      CallInst::Create(getASANCheckFunction(M), {ptrArg, sizeArg}, "", I);
    
  //blockMap[H]->getStatements().push_front(
  //        Stmt::assert_(E, {Attr::attr(Naming::LOOP_INVARIANT_ANNOTATION)}));
  //M.getStatements();

  //I->getParent()->emit(Stmt::assume(Expr::lit(false)));
  //auto ci= CallInst::Create(getASANCheckFunction(M), "", I);

  copyDbgMetadata(I, ptrArg);
  copyDbgMetadata(I, sizeArg);
  copyDbgMetadata(I, ci);
}

bool ASANChecker::runOnFunction(Function &F) {
  if (Naming::isSmackName(F.getName()) ||
      !SmackOptions::shouldCheckFunction(F.getName()))
    return false;

  this->visit(F);
  return true;
}

void ASANChecker::visitReturnInst(llvm::ReturnInst &I) {
//   auto &F = *I.getParent()->getParent();

//   if (SmackOptions::isEntryPoint(F.getName()))
//     insertMemoryLeakCheck(&I);

// Check if address is allocated
}

namespace {
Value *accessSizeAsPointer(Module &M, Value *V) {
  auto T = dyn_cast<PointerType>(V->getType());
  assert(T && "expected pointer type");

  return ConstantExpr::getIntToPtr(
      ConstantInt::get(
          Type::getInt64Ty(M.getContext()),
          M.getDataLayout().getTypeStoreSize(T->getPointerElementType())),
      PointerType::getUnqual(Type::getInt8Ty(M.getContext())));
}

Value *accessSizeAsPointer(LoadInst &I) {
  auto &M = *I.getParent()->getParent()->getParent();
  return accessSizeAsPointer(M, I.getPointerOperand());
}

Value *accessSizeAsPointer(StoreInst &I) {
  auto &M = *I.getParent()->getParent()->getParent();
  return accessSizeAsPointer(M, I.getPointerOperand());
}
} // namespace

void ASANChecker::visitLoadInst(LoadInst &I) {
  insertASANCheck(I.getPointerOperand(), accessSizeAsPointer(I), &I);
}

void ASANChecker::visitStoreInst(StoreInst &I) {
  insertASANCheck(I.getPointerOperand(), accessSizeAsPointer(I), &I);
}

void ASANChecker::visitMemSetInst(MemSetInst &I) {
  insertASANCheck(I.getDest(), I.getLength(), &I);
}

void ASANChecker::visitMemTransferInst(MemTransferInst &I) {
  insertASANCheck(I.getDest(), I.getLength(), &I);
  insertASANCheck(I.getSource(), I.getLength(), &I);
}

// Pass ID variable
char ASANChecker::ID = 0;
} // namespace smack
