#include "ast.h"
#include "generator.hpp"
#include "parser.hpp"

/*
    Modules containt functions
    Functions contains basic blocks
    Basic blocks contains instructions
*/

// Create LLVM module object
void GeneratorContext::compileModule(Block &root)
{
    std::cout << "Running code generation." << std::endl;

    // Argument types list for start function
    std::vector<llvm::Type *> argumentTypes;

    // Create void return type for function
    llvm::FunctionType *functionType = llvm::FunctionType::get(llvm::Type::getVoidTy(llvmContext), llvm::makeArrayRef(argumentTypes), false);

    // Create main function of given type.
    mainFunction = llvm::Function::Create(functionType, llvm::GlobalValue::InternalLinkage, "main", module);
    llvm::BasicBlock *block = llvm::BasicBlock::Create(llvmContext, "entry", mainFunction, 0);

    pushBlock(block);
    root.generateCode(*this);
    llvm::ReturnInst::Create(llvmContext, block);
    popBlock();

    llvm::legacy::PassManager passManager;
    passManager.add(llvm::createPrintModulePass(llvm::outs()));
    passManager.run(*module);
}

// Execute code
llvm::GenericValue GeneratorContext::runCode()
{
    // Process module with execution engine
    llvm::ExecutionEngine *executionEngine = llvm::EngineBuilder(std::unique_ptr<llvm::Module>(module)).create();
    executionEngine->finalizeObject();

    // Run code in main function
    std::vector<llvm::GenericValue> arguments;
    auto functionValue = executionEngine->runFunction(mainFunction, arguments);
    return functionValue;
}

// Return LLVM type from given identifier
static llvm::Type *typeOf(const Identifier &type)
{
    // TODO: Return string type on "String" identifier

    if (type.name.compare("Int") == 0)
        return llvm::Type::getInt64Ty(llvmContext);
    else if (type.name.compare("Double") == 0)
        return llvm::Type::getDoubleTy(llvmContext);

    return llvm::Type::getVoidTy(llvmContext);
}

// Return ConstantInt of specified integer.
llvm::Value *Integer::generateCode(GeneratorContext &context)
{
    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmContext), value, true);
}

// Return ConstantFP of specified integer.
llvm::Value *Double::generateCode(GeneratorContext &context)
{
    return llvm::ConstantFP::get(llvm::Type::getDoubleTy(llvmContext), value);
}

// Load variable identifier into memory
llvm::Value *Identifier::generateCode(GeneratorContext &context)
{
    if (context.locals().find(name) == context.locals().end())
    {
        std::cout << "Variable undeclared" << std::endl;
        return NULL;
    }

    return new llvm::LoadInst(context.locals()[name], "", false, context.currentBlock());
}

llvm::Value *MethodCall::generateCode(GeneratorContext &context)
{
    // Get function by name from module
    llvm::StringRef functionName = llvm::StringRef(id.name);
    llvm::Function *function = context.module->getFunction(functionName);
    
    if (function == NULL)
        std::cerr << "Function " << id.name.c_str() << " is undefined." << std::endl;

    std::vector<llvm::Value *> functionArguments;
    ExpressionList::const_iterator it;

    for (it = arguments.begin(); it != arguments.end(); it = it++)
        functionArguments.push_back((**it).generateCode(context));

    llvm::CallInst *functionCall = llvm::CallInst::Create(function, llvm::makeArrayRef(functionArguments), "", context.currentBlock());
    std::cout << "Created function " << id.name.c_str() << std::endl;
    return functionCall;
}

llvm::Value *BinaryOperator::generateCode(GeneratorContext &context)
{
    llvm::Instruction::BinaryOps instruction;

    switch (op)
    {
    // Arithmetic operators
    case PLUS_OP:
        instruction = llvm::Instruction::Add;
        break;
    case MINUS_OP:
        instruction = llvm::Instruction::Sub;
        break;
    case MUL_OP:
        instruction = llvm::Instruction::Mul;
        break;
    case DIV_OP:
        instruction = llvm::Instruction::SDiv;
        break;
    case MOD_OP:
        instruction = llvm::Instruction::SRem;
        break;
    default:
        return NULL;
        break;
    }

    return llvm::BinaryOperator::Create(instruction, lhs.generateCode(context),
                                        rhs.generateCode(context), "", context.currentBlock());
}

llvm::Value *Assignment::generateCode(GeneratorContext &context)
{
    if (context.locals().find(lhs.name) == context.locals().end())
    {
        std::cout << "Variable " << lhs.name << " is undeclared." << std::endl;
        return NULL;
    }

    // Save variable in memory.
    return new llvm::StoreInst(rhs.generateCode(context), context.locals()[lhs.name], false, context.currentBlock());
}

llvm::Value *Block::generateCode(GeneratorContext &context)
{
    StatementList::const_iterator it;
    llvm::Value *last = NULL;

    for (it = statements.begin(); it != statements.end(); it++)
    {
        std::cout << "Generating code for " << typeid(**it).name() << std::endl;
        last = (**it).generateCode(context);
    }

    std::cout << "Block created." << std::endl;
}

llvm::Value *ExpressionStatement::generateCode(GeneratorContext &context)
{
    std::cout << "Code for " << typeid(expression).name() << std::endl;
    return expression.generateCode(context);
}

llvm::Value *ReturnStatement::generateCode(GeneratorContext &context)
{
    std::cout << "Generating return code for " << typeid(returnExpression).name() << std::endl;
    llvm::Value *returnValue = returnExpression.generateCode(context);
    context.setCurrentReturnValue(returnValue);
    return returnValue;
}

llvm::Value *VariableDeclaration::generateCode(GeneratorContext &context)
{
    unsigned int addressSpace = 64;
    const llvm::Twine typeName = llvm::Twine(type.name.c_str());
 
    std::cout << "Declaring variable [" << id.name << "] of type [" << type.name << "]" << std::endl;
    llvm::AllocaInst *allocationInstance = new llvm::AllocaInst(typeOf(type), addressSpace, typeName, context.currentBlock());
    context.locals()[id.name] = allocationInstance;

    // If declared variable is assigned to something
    if (assignmentExpression != NULL)
    {
        Assignment assignment(id, *assignmentExpression);
        assignment.generateCode(context);
    }

    return allocationInstance;
}

llvm::Value *FunctionDeclaration::generateCode(GeneratorContext &context)
{
    const llvm::Twine functionName = llvm::Twine(id.name.c_str());
    std::vector <llvm::Type*> argumentTypes;
    VariableList::const_iterator it;

    for (it = arguments.begin(); it != arguments.end(); it++)
        argumentTypes.push_back(typeOf((**it).type));

    llvm::FunctionType *functionType = llvm::FunctionType::get(typeOf(type), llvm::makeArrayRef(argumentTypes), false);
    llvm::Function *function = llvm::Function::Create(functionType, llvm::GlobalValue::InternalLinkage, functionName, context.module);
    llvm::BasicBlock *basicBlock = llvm::BasicBlock::Create(llvmContext, "entry", function, 0);

    context.pushBlock(basicBlock);

    llvm::Function::arg_iterator argumentValues = function->arg_begin();
    llvm::Value *argumentValue;

    for (it = arguments.begin(); it != arguments.end(); it++)
    {
        (**it).generateCode(context);
        argumentValue = &*argumentValues++;
        argumentValue->setName((*it)->id.name.c_str());
        llvm::StoreInst *storeInstance = new llvm::StoreInst(argumentValue, context.locals()[(*it)->id.name], false, basicBlock);
    }

    block.generateCode(context);
    llvm::ReturnInst::Create(llvmContext, context.getCurrentReturnValue(), basicBlock);
    context.popBlock();

    std::cout << "Created function " << id.name << std::endl;
    return function;
}