#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdio>

using namespace llvm;
using namespace llvm::orc;

int main() {
    // Initialize LLVM native target
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    auto context = std::make_unique<LLVMContext>();
    auto module = std::make_unique<Module>("printf_module", *context);
    IRBuilder<> builder(*context);

    // Declare printf
    FunctionType *printfType = FunctionType::get(
        builder.getInt32Ty(), builder.getInt8PtrTy(), true);
    FunctionCallee printfFunc = module->getOrInsertFunction("printf", printfType);

    // Create main function
    FunctionType *mainType = FunctionType::get(builder.getInt32Ty(), false);
    Function *mainFunc = Function::Create(
        mainType, Function::ExternalLinkage, "main", module.get());
    BasicBlock *entry = BasicBlock::Create(*context, "entry", mainFunc);
    builder.SetInsertPoint(entry);

    // Create format string
    Value *formatStr = builder.CreateGlobalStringPtr("Hello from LLVM!\n");

    // Call printf
    builder.CreateCall(printfFunc, formatStr);
    builder.CreateRet(builder.getInt32(0));

    // Build JIT
    auto jit = cantFail(LLJITBuilder().create());

    // Add external symbol for printf
    cantFail(jit->getMainJITDylib().define(
        absoluteSymbols({
            {jit->mangleAndIntern("printf"),
             JITEvaluatedSymbol(pointerToJITTargetAddress(&printf),
                                JITSymbolFlags::Exported)}}
        )));

    // Add module
    cantFail(jit->addIRModule(ThreadSafeModule(std::move(module), std::move(context))));

    // Lookup main function
    auto sym = cantFail(jit->lookup("main"));

    // Call main
    int (*mainFn)() = (int (*)())(intptr_t)sym.getAddress();
    return mainFn();
}
