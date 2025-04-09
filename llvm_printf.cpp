#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/ExecutionEngine/Orc/MangleAndInterner.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

#include <cstdio>

using namespace llvm;
using namespace llvm::orc;

int main() {
    // Initialize LLVM
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    // Create LLVM context and module
    auto context = std::make_unique<LLVMContext>();
    auto module = std::make_unique<Module>("my_module", *context);
    IRBuilder<> builder(*context);

    // Declare printf
    FunctionType *printfType = FunctionType::get(builder.getInt32Ty(), builder.getInt8PtrTy(), true);
    FunctionCallee printfFunc = module->getOrInsertFunction("printf", printfType);

    // Create main function
    FunctionType *mainType = FunctionType::get(builder.getInt32Ty(), false);
    Function *mainFunc = Function::Create(mainType, Function::ExternalLinkage, "main", module.get());
    BasicBlock *entry = BasicBlock::Create(*context, "entry", mainFunc);
    builder.SetInsertPoint(entry);

    // Call printf("Hello from LLVM!\n");
    Value *msg = builder.CreateGlobalStringPtr("Hello from LLVM!\n");
    builder.CreateCall(printfFunc, {msg});
    builder.CreateRet(builder.getInt32(0));

    // Create the JIT
    auto jit = cantFail(LLJITBuilder().create());

    // Mangle symbol and define printf manually
    MangleAndInterner mangle(jit->getExecutionSession(), jit->getDataLayout());
    auto symbol = mangle("printf");

    cantFail(jit->getMainJITDylib().define(absoluteSymbols({
        {symbol, JITEvaluatedSymbol(pointerToJITTargetAddress(&printf),
                                    JITSymbolFlags::Exported)}
    })));

    // Add module
    cantFail(jit->addIRModule(ThreadSafeModule(std::move(module), std::move(context))));

    // Look up 'main' symbol
    auto mainSym = cantFail(jit->lookup("main"));
    auto *mainFn = (int (*)())(intptr_t)mainSym.getAddress();

    return mainFn();
}
