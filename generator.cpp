#include "ast.h"
#include "generator.hpp"
#include "parser.hpp"
#include <algorithm>

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

    pushBlock(block, "Main function basic block");
    root.generateCode(*this);
    llvm::ReturnInst::Create(llvmContext, this->currentBlock());
    popBlock();

    llvm::legacy::PassManager passManager;

    if (verboseOutput)
        passManager.add(llvm::createPrintModulePass(llvm::outs()));

    std::cout << "Compiled successfully." << std::endl
              << std::endl;
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
    else if (type.name.compare("String") == 0)
        return llvm::Type::getInt8PtrTy(llvmContext);

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
    // TODO: This mess...
    // #################################################################################
    size_t pos = value.find("\\n");
    if (pos != std::string::npos)
    {
        value.replace(pos, 2, 1, '\n');
    }
    pos = value.find("\"");
    value.erase(pos, 1);
    pos = value.find("\"");
    value.erase(pos, 1);

    const char *constValue = value.c_str();

    llvm::Constant *format_const =
        llvm::ConstantDataArray::getString(llvmContext, constValue);
    llvm::GlobalVariable *var =
        new llvm::GlobalVariable(
            *context.module, llvm::ArrayType::get(llvm::IntegerType::get(llvmContext, 8), strlen(constValue) + 1),
            true, llvm::GlobalValue::PrivateLinkage, format_const, ".str");

    llvm::Constant *zero =
        llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(llvmContext));

    std::vector<llvm::Constant *> indices;
    indices.push_back(zero);
    indices.push_back(zero);
    llvm::Constant *var_ref = llvm::ConstantExpr::getGetElementPtr(
        llvm::ArrayType::get(llvm::IntegerType::get(llvmContext, 8), strlen(constValue) + 1),
        var, indices);

    // #######################################################################################
    return var_ref;
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
    llvm::IRBuilder<> builder(context.currentBlock()); // TODO: Use builder instead of CallInst

    // For called function
    std::vector<llvm::Value *> functionArguments;
    ExpressionList::const_iterator it;

    if (function == NULL)
    {
        if (id.name.compare("print") != 0)
        {
            std::cerr << "Function " << id.name.c_str() << " is undefined." << std::endl;
            return NULL;
        }
        else
        {
            llvm::Constant *printFunction = context.module->getOrInsertFunction("printf", llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(llvmContext), llvm::PointerType::get(llvm::Type::getInt8Ty(llvmContext), 0), true));

            for (it = arguments.begin(); it != arguments.end(); it++)
                functionArguments.push_back((**it).generateCode(context));

            return builder.CreateCall(printFunction, functionArguments, "printfCall");
        }
    }

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
    llvm::Value *rhsValue = rhs.generateCode(context);

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
    llvm::Value *invertedValue = NULL;

    if (dynamic_cast<Integer *>(&rhs))
    {
        Integer *integer = dynamic_cast<Integer *>(&rhs);
        int64_t reverse = 0;
        int64_t rem;

        while (integer->value != 0)
        {
            rem = integer->value % 10;
            reverse = reverse * 10 + rem;
            integer->value /= 10;
        }

        integer->value = reverse;
        return integer->generateCode(context);
    }
    else if (dynamic_cast<String *>(&rhs))
    {
        String *string = dynamic_cast<String *>(&rhs);
        std::reverse(string->value.begin(), string->value.end());
        return string->generateCode(context);
    }

    return invertedValue;
}

llvm::Value *Assignment::generateCode(GeneratorContext &context)
{   std::cout <<"Reached here4\n";
    if (context.locals().find(lhs.name) == context.locals().end())
    {std::cout <<"Reached here5\n";
        std::cerr << "Variable " + lhs.name + " is undeclared." << std::endl;
        return NULL;
    }
    std::cout <<"Reached here6\n";
    context.currentBlock()->print(llvm::outs());
    std::cout <<"Reached here7\n";
    auto a = rhs.generateCode(context);
    std::cout <<"Reached here8\n";
    auto b = context.locals()[lhs.name];
    std::cout <<"Reached here9\n";
    a->print(llvm::outs());
    b->print(llvm::outs());
    // Save variable in memory.
    return new llvm::StoreInst(a, b, false, context.currentBlock());
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

    context.logMessage("Declaring variable [" + id.name + "] of type [" + type.name + "]");
    context.module->print(llvm::outs(), nullptr);
    llvm::AllocaInst *allocationInstance = new llvm::AllocaInst(typeOf(type), addressSpace, typeName, context.currentBlock());
    
    context.locals()[id.name] = allocationInstance;

    // If declared variable is assigned to something
    if (assignmentExpression != NULL)
    {
        std::cout <<"Reached here\n";
        Assignment assignment(id, *assignmentExpression);
        std::cout <<"Reached here1\n";
        assignment.generateCode(context);
        std::cout <<"Reached here2\n";
    }
std::cout <<"Reached here3\n";
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

    context.pushBlock(basicBlock, "Basic function block");

    llvm::Function::arg_iterator argumentValues = function->arg_begin();
    llvm::Value *argumentValue;

    for (it = arguments.begin(); it != arguments.end(); it++)
    {
        context.module->print(llvm::outs(), nullptr);
        //function->print(llvm::outs());
        (**it).generateCode(context);
        argumentValue = &*argumentValues++;
        argumentValue->setName((*it)->id.name.c_str());
        llvm::StoreInst *storeInstance = new llvm::StoreInst(argumentValue, context.locals()[(*it)->id.name], false, basicBlock);
        
    }

    block.generateCode(context);
    llvm::ReturnInst::Create(llvmContext, context.getCurrentReturnValue(), context.currentBlock());
    context.popBlock();

    context.logMessage("Created function " + id.name);
    return function;
}

llvm::Value *Conditional::generateCode(GeneratorContext &context)
{
    llvm::IRBuilder<> builder(llvmContext);
    llvm::Function *function = context.currentBlock()->getParent();
    std::map<std::string, llvm::Value *> locals = context.currentBlockLocals();

    // Comparison result
    llvm::Value *conditionValue = comparison->generateCode(context);

    // Blocks for branches
    llvm::BasicBlock *thenBlock = llvm::BasicBlock::Create(llvmContext, "then", function);
    llvm::BasicBlock *elseBlock = llvm::BasicBlock::Create(llvmContext, "else");
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(llvmContext, "ifcont");
    
    // Insert if block to function
    function->getBasicBlockList().push_back(thenBlock);

    if (elseBlockNode != nullptr)
        llvm::BranchInst::Create(thenBlock, elseBlock, conditionValue, context.currentBlock());
    else
        llvm::BranchInst::Create(thenBlock, mergeBlock, conditionValue, context.currentBlock());

    // To match variables
    context.pushBlock(thenBlock, "Then Block", locals);
    llvm::Value *thenValue = thenBlockNode->generateCode(context);
    std::cout << "Got here 10\n";
    if(context.getCurrentReturnValue() != nullptr)
        llvm::ReturnInst::Create(llvmContext, context.getCurrentReturnValue(), context.currentBlock());
    else
        llvm::BranchInst::Create(mergeBlock, context.currentBlock());
    context.popBlock();

    if (elseBlockNode != nullptr)
    {
        function->getBasicBlockList().push_back(elseBlock);
        context.pushBlock(elseBlock, "Else block", locals);
        llvm::Value *elseValue = elseBlockNode->generateCode(context);
        if(context.getCurrentReturnValue() != nullptr)
            llvm::ReturnInst::Create(llvmContext, context.getCurrentReturnValue(), context.currentBlock());
        else
            llvm::BranchInst::Create(mergeBlock, context.currentBlock());
        context.popBlock();
    }

    function->getBasicBlockList().push_back(mergeBlock);

    //get locals from block before if
    locals = context.currentBlockLocals();
    context.setCurrentBlock(mergeBlock, "Merge block", locals);
    //context.pushBlock(mergeBlock, "Merge block", locals);
    //llvm::ReturnInst::Create(llvmContext, context.getCurrentReturnValue(), context.currentBlock());
    //context.popBlock();

    return NULL;
}