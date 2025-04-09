#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/DynamicLibrary.h"  // Include this header

using namespace llvm;
using namespace llvm::orc;

int main() {
    // Initialize LLVM
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    
    // Create context, module, builder
    auto context = std::make_unique<LLVMContext>();
    auto module = std::make_unique<Module>("my_module", *context);
    IRBuilder<> builder(*context);

    // Declare printf function (int printf(char*, ...))
    std::vector<Type*> printfArgs = { builder.getInt8PtrTy() };
    FunctionType *printfType = FunctionType::get(builder.getInt32Ty(), printfArgs, true);
    FunctionCallee printfFunc = module->getOrInsertFunction("printf", printfType);

    // Create main function: int main()
    FunctionType *mainType = FunctionType::get(builder.getInt32Ty(), false);
    Function *mainFunc = Function::Create(mainType, Function::ExternalLinkage, "main", module.get());
    BasicBlock *entry = BasicBlock::Create(*context, "entry", mainFunc);
    builder.SetInsertPoint(entry);

    // Create format string and call printf
    Value *msg = builder.CreateGlobalStringPtr("Hello from LLVM!\n");
    builder.CreateCall(printfFunc, { msg });
    builder.CreateRet(builder.getInt32(0));

    // --------- Add this line to load host symbols ---------
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
    // -------------------------------------------------------

    // Create JIT
    auto jit = LLJITBuilder().create();
    if (!jit) {
        llvm::errs() << "Error creating JIT\n";
        return 1;
    }

    // Add the module to the JIT
    if (auto err = (*jit)->addIRModule(ThreadSafeModule(std::move(module), std::move(context)))) {
        llvm::errs() << "Error adding module to JIT\n";
        return 1;
    }

    // Look up the "main" function
    auto sym = (*jit)->lookup("main");
    if (!sym) {
        llvm::errs() << "Error finding 'main'\n";
        return 1;
    }

    // Cast and call the JIT'ed main function
    using MainFn = int (*)();
    auto *mainPtr = (MainFn)(intptr_t)(sym->getAddress());
    return mainPtr();
}
