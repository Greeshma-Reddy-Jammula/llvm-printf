#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

int main() {
    LLVMContext Context;
    Module *module = new Module("printf_example", Context);
    IRBuilder<> builder(Context);

    // Declare the printf function: int printf(const char*, ...)
    std::vector<Type *> printf_arg_types;
    printf_arg_types.push_back(Type::getInt8PtrTy(Context)); // const char*
    FunctionType *printf_type = FunctionType::get(
        IntegerType::getInt32Ty(Context), printf_arg_types, true);
    FunctionCallee printf_func = module->getOrInsertFunction("printf", printf_type);

    // Create a main function: int main()
    FunctionType *main_type = FunctionType::get(
        Type::getInt32Ty(Context), false);
    Function *main_func = Function::Create(
        main_type, Function::ExternalLinkage, "main", module);

    // Create a basic block and set the insertion point
    BasicBlock *entry = BasicBlock::Create(Context, "entrypoint", main_func);
    builder.SetInsertPoint(entry);

    // Create format string
    Value *format_str = builder.CreateGlobalStringPtr("Hello, LLVM!\n");

    // Call printf(format_str)
    builder.CreateCall(printf_func, format_str);

    // Return 0
    builder.CreateRet(ConstantInt::get(Context, APInt(32, 0)));

    // Verify the function
    verifyFunction(*main_func);

    // Print out the LLVM IR
    module->print(outs(), nullptr);

    delete module;
    return 0;
}
