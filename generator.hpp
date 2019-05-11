#include <stack>
#include <llvm-7/llvm/IR/Module.h>
#include <llvm-7/llvm/IR/LLVMContext.h>
#include <llvm-7/llvm/ExecutionEngine/GenericValue.h>
#include <llvm-7/llvm/IR/Instructions.h>
#include <llvm-7/llvm/IR/LegacyPassManager.h>
#include <llvm-7/llvm/IR/IRPrintingPasses.h>
#include <llvm-7/llvm/ExecutionEngine/ExecutionEngine.h>

class Block;
static llvm::LLVMContext llvmContext;

/**
 * Code block which contains basi block type from LLVM,
 * return value and map of local variables.
 */
class GeneratorBlock
{
public:
    llvm::BasicBlock *block;
    llvm::Value *returnValue;
    std::map<std::string, llvm::Value *> locals;
};

class GeneratorContext
{
    std::stack<GeneratorBlock *> blocks;
    llvm::Function *mainFunction;

public:
    llvm::Module *module;

    GeneratorContext()
    {
        module = new llvm::Module("main", llvmContext);
    }

    void compileModule(Block &root);

    llvm::GenericValue runCode();

    std::map<std::string, llvm::Value *> &locals()
    {
        return blocks.top()->locals;
    }

    llvm::BasicBlock *currentBlock()
    {
        return blocks.top()->block;
    }

    void pushBlock(llvm::BasicBlock *block)
    {
        GeneratorBlock *generatorBlock = new GeneratorBlock();
        blocks.push(generatorBlock);
        blocks.top()->returnValue = NULL;
        blocks.top()->block = block;
    }

    void popBlock()
    {
        GeneratorBlock *top = blocks.top();
        blocks.pop();
        delete top;
    }

    void setCurrentReturnValue(llvm::Value *value)
    {
        blocks.top()->returnValue = value;
    }

    llvm::Value* getCurrentReturnValue() 
    {
        return blocks.top()->returnValue;
    }
};