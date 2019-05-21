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

    pushBlock(block);
    root.generateCode(*this);
    llvm::ReturnInst::Create(llvmContext, block);
    popBlock();

    llvm::legacy::PassManager passManager;

    if (verboseOutput)
        passManager.add(llvm::createPrintModulePass(llvm::outs()));

    std::cout << "Compiled successfully." << std::endl << std::endl;
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
    //conditionValue = builder.CreateICmpNE(conditionValue, builder.getInt1(false), "ifcond");
    llvm::Function *function = context.currentBlock()->getParent();

    // Blocks for branches
    llvm::BasicBlock *thenBlock = llvm::BasicBlock::Create(llvmContext, "then", function);
    llvm::BasicBlock *elseBlock = llvm::BasicBlock::Create(llvmContext, "else");
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(llvmContext, "ifcont");

    // Insert if block to function
    function->getBasicBlockList().push_back(thenBlock);
    llvm::Value *conditionValue = comparison->generateCode(context);

    if (elseBlockNode != nullptr)
        llvm::BranchInst::Create(thenBlock, elseBlock, conditionValue, context.currentBlock());
    else
        llvm::BranchInst::Create(thenBlock, mergeBlock, conditionValue, context.currentBlock());

    // To matc variables
    context.pushBlock(thenBlock);
    llvm::Value *thenValue = thenBlockNode->generateCode(context);
    llvm::BranchInst::Create(mergeBlock, context.currentBlock());
    context.popBlock();

    if (elseBlockNode != nullptr)
    {
        function->getBasicBlockList().push_back(elseBlock);
        context.pushBlock(elseBlock);
        llvm::Value *elseValue = elseBlockNode->generateCode(context);
        llvm::BranchInst::Create(mergeBlock, context.currentBlock());
        context.popBlock();
    }

    function->getBasicBlockList().push_back(mergeBlock);
    context.pushBlock(mergeBlock);
    llvm::ReturnInst::Create(llvmContext, context.getCurrentReturnValue(), context.currentBlock());

    std::cout << "----" << std::endl;

    //builder.CreateCondBr(conditionValue, ifBlock, elseBlock);
    //builder.SetInsertPoint(elseBlock);
    //builder.CreateCondBr(condition, ifBlock, elseBlock);
    //builder.SetInsertPoint(elseBlock);
    //builder.CreateCondBr(comparisonResult, ifBlock, elseBlock,);

    //Generate condition value code
    //llvm::Value *conditionValue = comparison->generateCode(context);

    // If condition converted to bool value
    //llvm::Value* conditionValue = builder.CreateICmpNE(builder.getInt1(false), builder.getInt1(false), "ifcondition");

    // -1 => true, 0  => false
    //

    // Pinter to function where if is created

    // Blocks

    // Create conditional instruction
    //builder.CreateCondBr(conditionValue, ifBlock, elseBlock);
    //builder.SetInsertPoint(ifBlock);

    //llvm::Value *thenValue = onTrue->generateCode(context);

    //if(!thenValue)
    //    return nullptr;

    // Emit instrucion: "jump to mergeBlock (block executed after then and else)"
    //builder.CreateBr(mergeBlock);

    //Code generation of ifBlock can change current block, need to update.
    //ifBlock = builder.GetInsertBlock();

    //Now insert else block
    //function->getBasicBlockList().push_back(elseBlock);
    //builder.SetInsertPoint(elseBlock);

    //Generate else block code
    //llvm::Value *elseValue = onFalse->generateCode(context);

    //if(!elseValue)
    //    return nullptr;

    //Create jmp to merge block instruction
    //builder.CreateBr(mergeBlock);

    //Code generation of elseBlock can change current block, need to update
    //elseBlock = builder.GetInsertBlock();

    //Emit mergeBlock
    //function->getBasicBlockList().push_back(mergeBlock);
    //builder.SetInsertPoint(mergeBlock);

    //std::cout << "Creating PHINode \n";
    //TODO: fix this (value types dont match in CreatePHI)
    //llvm::PHINode *phiNode = builder.CreatePHI(llvm::Type::getInt64Ty(llvmContext), 2, "iftemp");

    //std::cout << "Adding PHINode values\n";

    //phiNode->addIncoming(thenValue, ifBlock);
    //std::cout << "Added then value\n";

    //phiNode->addIncoming(elseValue, elseBlock);
    //llvm::Value *result = builder.CreateICmpEQ(conditionValue, toCompare);
    //if (llvm::ConstantInt* CI = llvm::dyn_cast<llvm::ConstantInt>(conditionValue)) {

    //}
    //} else {
    //    std::cout << "Nice try..." << std::endl;
    //}
    //std::cout << "Returning PHINode\n";

    return NULL;
}

/*
llvm::ConstantInt* e = llvm::dyn_cast<llvm::ConstantInt>(comparisonResult);
*/