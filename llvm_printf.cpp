#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::orc;

int main() {
    // Initialize LLVM backend
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    // Create context, module, and builder
    auto context = std::make_unique<LLVMContext>();
    auto module = std::make_unique<Module>("my_module", *context);
    IRBuilder<> builder(*context);

    // Declare printf (int printf(char*, ...))
    std::vector<Type*> printfArgs = { builder.getInt8PtrTy() };
    FunctionType *printfType = FunctionType::get(builder.getInt32Ty(), printfArgs, true);
    FunctionCallee printfFunc = module->getOrInsertFunction("printf", printfType);

    // Define main function (int main())
    FunctionType *mainType = FunctionType::get(builder.getInt32Ty(), false);
    Function *mainFunc = Function::Create(mainType, Function::ExternalLinkage, "main", module.get());
    BasicBlock *entry = BasicBlock::Create(*context, "entry", mainFunc);
    builder.SetInsertPoint(entry);

    // Create string and call printf
    Value *msg = builder.CreateGlobalStringPtr("Hello from LLVM!\n");
    builder.CreateCall(printfFunc, { msg });

    // Return 0
    builder.CreateRet(builder.getInt32(0));

    // Optional: print IR for debug
    // module->print(outs(), nullptr);

    // JIT compile and run
    auto jit = LLJITBuilder().create();
    if (!jit) return 1;

    if (auto err = (*jit)->addIRModule(ThreadSafeModule(std::move(module), std::move(context)))) return 1;

    auto sym = (*jit)->lookup("main");
    if (!sym) return 1;

    using MainFn = int (*)();
    auto *mainPtr = (MainFn)(intptr_t)(sym->getAddress());
    return mainPtr();
}
