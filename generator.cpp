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
    this->logMessage("Running code generation.");

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

    if (verboseOutput)
        passManager.add(llvm::createPrintModulePass(llvm::outs()));

    std::cout << "Compiled successfully." << std::endl;
    passManager.run(*module);
}

// Execute code
llvm::GenericValue GeneratorContext::runCode()
{
    this->logMessage("Running code.");

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

llvm::Value *String::generateCode(GeneratorContext &context)
{
    std::cout << "string" << std::endl;
    return NULL;
}

// Load variable identifier into memory
llvm::Value *Identifier::generateCode(GeneratorContext &context)
{
    if (context.locals().find(name) == context.locals().end())
    {
        std::cerr << "Variable " << name << " is undeclared." << std::endl;
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
    {
        if (id.name.compare("print") != 0)
        {
            std::cerr << "Function " << id.name.c_str() << " is undefined." << std::endl;
            return NULL;
        }
        else
        {
            // Get or instert print function
        }
    }

    std::vector<llvm::Value *> functionArguments;
    ExpressionList::const_iterator it;

    for (it = arguments.begin(); it != arguments.end(); it++)
        functionArguments.push_back((**it).generateCode(context));

    llvm::CallInst *functionCall = llvm::CallInst::Create(function, llvm::makeArrayRef(functionArguments), "", context.currentBlock());
    context.logMessage("Created function " + id.name);
    return functionCall;
}

llvm::Value *BinaryOperator::generateCode(GeneratorContext &context)
{
    llvm::Instruction::BinaryOps instruction;
    llvm::IRBuilder<> builder(context.currentBlock());
    llvm::Value *lhsValue = lhs.generateCode(context);
    llvm::Value *rhsValue = lhs.generateCode(context);

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
    // Comparison operators
    case EQ:
        return builder.CreateICmpEQ(lhsValue, rhsValue);
    case LT:
        return builder.CreateICmpSLT(lhsValue, rhsValue);
    case GT:
        return builder.CreateICmpSGT(lhsValue, rhsValue);
    case LTE:
        return builder.CreateICmpSLE(lhsValue, rhsValue);
    case GTE:
        return builder.CreateICmpSGE(lhsValue, rhsValue);
    case NEQ:
        return builder.CreateICmpNE(lhsValue, rhsValue);
    default:
        return NULL;
        break;
    }

    return llvm::BinaryOperator::Create(instruction, lhsValue, rhsValue, "", context.currentBlock());
}

llvm::Value *UnaryOperator::generateCode(GeneratorContext &context)
{
    uint addressSpace = 64;
    uint64_t value = 1;
    llvm::ConstantInt *one = llvm::ConstantInt::get(llvmContext, llvm::APInt(addressSpace, value, false));
    llvm::Instruction::BinaryOps instruction;

    switch (op)
    {
    case INC_OP:
        instruction = llvm::Instruction::Add;
        break;
    case DEC_OP:
        instruction = llvm::Instruction::Sub;
        break;
    default:
        return NULL;
        break;
    }

    return llvm::BinaryOperator::Create(instruction, exp.generateCode(context), one, "", context.currentBlock());
}

llvm::Value *InversionOperator::generateCode(GeneratorContext &context)
{
    /*
    if(op == INVERSE_OP) {
        std::cout << "Inversion is not implemented yet." << std::endl;
        std::cout << rhs.name << std::endl;
        llvm::Value *value = context.locals().find(rhs.name)->second;
        
        std::string v = value->getName();
        std::cout << v << std::endl;

        if(llvm::IntegerType* cInt = llvm::dyn_cast<llvm::IntegerType>(value))
        {
            std::cout << "Yes" << std::endl;
        } else {
            std::cout << "No" << std::endl;
        }
    }
    */

    return NULL;
}

llvm::Value *Assignment::generateCode(GeneratorContext &context)
{
    if (context.locals().find(lhs.name) == context.locals().end())
    {
        std::cerr << "Variable " + lhs.name + " is undeclared." << std::endl;
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
        std::string statementName = typeid(**it).name();
        context.logMessage("Generating code for " + statementName);
        last = (**it).generateCode(context);
    }

    context.logMessage("Block created.");
}

llvm::Value *ExpressionStatement::generateCode(GeneratorContext &context)
{
    std::string expressionName = typeid(expression).name();
    context.logMessage("Generating code for expression " + expressionName);
    return expression.generateCode(context);
}

// Generates code for return statement
llvm::Value *ReturnStatement::generateCode(GeneratorContext &context)
{
    std::string returnName = typeid(returnExpression).name();
    context.logMessage("Generating return code for " + returnName);

    llvm::Value *returnValue = returnExpression.generateCode(context);
    context.setCurrentReturnValue(returnValue);
    return returnValue;
}

// Generates code for variable declaration
llvm::Value *VariableDeclaration::generateCode(GeneratorContext &context)
{
    unsigned int addressSpace = 64;
    const llvm::Twine typeName = llvm::Twine(type.name.c_str());

    if (type.name.compare("String") == 0)
    {
        // TODO: generate code for string (char*) value
        return NULL;
    }

    context.logMessage("Declaring variable [" + id.name + "] of type [" + type.name + "]");
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

// Generates code for function declaration
llvm::Value *FunctionDeclaration::generateCode(GeneratorContext &context)
{
    const llvm::Twine functionName = llvm::Twine(id.name.c_str());
    std::vector<llvm::Type *> argumentTypes;
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

    context.logMessage("Created function " + id.name);
    return function;
}

llvm::Value *Conditional::generateCode(GeneratorContext &context)
{
    std::cout << "Yes, this is an if statement." << std::endl;
    return NULL;
}