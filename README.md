This project demonstrates how to use the **LLVM C++ API** to generate LLVM Intermediate Representation (IR) code that prints `"Hello, LLVM!"` using `printf`.

---

## 🧠 What does this code do?

It **programmatically builds LLVM IR** using C++ and the LLVM library. The IR simulates a C-like program with a `main()` function that calls `printf("Hello, LLVM!\n")`.

---

## 🔧 Technical Summary 

- **Creates an LLVM context**: All LLVM objects are tied to a context.
- **Creates a module**: A container for functions and global variables.
- **Declares the `printf` function**: Adds a variadic external function signature.
- **Creates a `main` function**: The entry point of our IR program.
- **Builds a basic block**: The first code block inside `main`.
- **Adds a global format string**: Equivalent to `const char*`.
- **Calls `printf`** with the format string.
- **Returns `0`** from `main`.
- **Verifies and prints the LLVM IR** to the terminal.




### 🔢 **Full Line-by-Line Code Explanation**


                    #include "llvm/IR/IRBuilder.h"
                    #include "llvm/IR/LLVMContext.h"
                    #include "llvm/IR/Module.h"
                    #include "llvm/IR/Verifier.h"
                    #include "llvm/Support/raw_ostream.h"

- ✅ These are LLVM header files that let you create and manipulate LLVM IR.
- `IRBuilder.h`: Simplifies generating instructions.
- `LLVMContext.h`: Manages memory and unique types across LLVM structures.
- `Module.h`: Represents a module (a full LLVM IR program).
- `Verifier.h`: Checks the correctness of the IR.
- `raw_ostream.h`: Used to print LLVM IR to the console.

                    using namespace llvm;
- ✅ This avoids writing `llvm::` before every LLVM type (like `llvm::Function`, `llvm::Module`, etc.)

                    int main() {
- ✅ This is the starting point of the C++ program — just like `int main()` in C. Everything will happen inside this block.




                    LLVMContext Context;

- ✅ Creates a new **LLVMContext**.  
- Every LLVM object (module, type, instruction) is tied to a context.
- Think of this as the workspace or environment for IR creation.

                     Module *module = new Module("printf_example", Context);

- ✅ Creates a new **LLVM module** named `"printf_example"`.
- A module is like a C/C++ source file: it contains functions, global variables, etc.
- This module will contain our IR.
            
                    IRBuilder<> builder(Context);

 - ✅ `IRBuilder` helps in **creating LLVM instructions** easily.
 - It handles insertion points and provides helper methods like `CreateCall`, `CreateRet`, etc.

                    std::vector<Type *> printf_arg_types;

- ✅ Declares a list (vector) to hold the argument types of the `printf` function.
 - `printf` expects at least one argument: a string (`const char*`), and possibly more.

                     printf_arg_types.push_back(Type::getInt8PtrTy(Context)); // const char*

- ✅ Adds the first argument type: `i8*` (pointer to 8-bit integer), which corresponds to `char*` in C.
- This is the type for a format string like `"Hello, LLVM!\n"`.

                    FunctionType *printf_type = FunctionType::get(
                       IntegerType::getInt32Ty(Context), printf_arg_types, true);


- ✅ Defines the type of the `printf` function:
- Return type: `int` → `i32` in LLVM.
- Argument types: one `i8*` (the format string).
- `true`: Marks the function as **variadic** — meaning it can accept more arguments (like `printf("Hello %s", name)`).

                     FunctionCallee printf_func = module->getOrInsertFunction("printf", printf_type);

- ✅ Declares the `printf` function in the module.
- We are not defining `printf`, just saying “Hey, this function exists — someone else (like C runtime) will define it.”
- This allows us to **call it later** in IR.

                    FunctionType *main_type = FunctionType::get(
                         Type::getInt32Ty(Context), false);          

- ✅ Defines the `main()` function type:
- Return type: `int` → `i32`
- Takes **no arguments** (`false`).

                    Function *main_func = Function::Create(
                        main_type, Function::ExternalLinkage, "main", module);

 - ✅ Creates the `main()` function inside the LLVM module:
- `ExternalLinkage`: This function is accessible outside the module.
- `"main"`: The name of the function.
- `module`: The module where it should be added.

                     BasicBlock *entry = BasicBlock::Create(Context, "entrypoint", main_func);


- ✅ Creates the first **basic block** inside `main`.
- A basic block is a sequence of instructions with **no jumps** in the middle.
- `"entrypoint"` is just a name. It’s like labeling the first block of `main()`.

                     builder.SetInsertPoint(entry);

- ✅ Tells the `IRBuilder` to insert all further instructions into this `entry` block.


                    Value *format_str = builder.CreateGlobalStringPtr("Hello, LLVM!\n");

          
- ✅ Adds a global string to the module: `"Hello, LLVM!\n"`
- It returns a `Value*` representing a pointer to the start of the string.
- This is equivalent to: `const char* str = "Hello, LLVM!\n";` in C.

                    builder.CreateCall(printf_func, format_str);

- ✅ Generates an LLVM `call` instruction:
- Calls the `printf` function with our format string.


                    builder.CreateRet(ConstantInt::get(Context, APInt(32, 0)));

- ✅ Returns `0` from the `main` function.
- `ConstantInt::get(...)` creates an LLVM `i32` constant with value 0.
- Equivalent to: `return 0;` in C/C++.

                    verifyFunction(*main_func);
          
- ✅ Verifies that the function we just built is **valid LLVM IR**.
- This is important for debugging; it checks things like incorrect return types, bad call instructions, etc.

                    module->print(outs(), nullptr);

  - ✅ Prints the generated LLVM IR to the terminal (`stdout`).
  - You’ll see IR similar to what Clang or other frontends generate.

                    delete module;

  - ✅ Frees the memory allocated for the module.

                    return 0;
                        }
- ✅ End of the `main()` function. Returns 0 to indicate successful execution.

