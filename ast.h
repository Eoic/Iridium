#include <iostream>
#include <vector>
#include <llvm-7/llvm/IR/Value.h>

class GeneratorContext;
class Statement;
class Expression;
class VariableDeclaration;

typedef std::vector<Statement *> StatementList;
typedef std::vector<Expression *> ExpressionList;
typedef std::vector<VariableDeclaration *> VariableList;

class Node
{
public:
    virtual ~Node() {}
    virtual llvm::Value *generateCode(GeneratorContext &context){};
};

class Expression : public Node
{
};

class Statement : public Node
{
};

class Integer : public Expression
{
public:
    long long value;
    Integer(long long value) : value(value) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class Double : public Expression
{
public:
    double value;
    Double(double value) : value(value) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class String : public Expression
{
public:
    std::string value;
    String(std::string value) : value(value) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class Identifier : public Expression
{
public:
    std::string name;
    Identifier(const std::string name) : name(name) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class MethodCall : public Expression
{
public:
    const Identifier &id;
    ExpressionList arguments;
    MethodCall(const Identifier &id) : id(id) {}
    MethodCall(const Identifier &id, ExpressionList &arguments) : id(id), arguments(arguments) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class BinaryOperator : public Expression
{
public:
    int op;
    Expression &lhs;
    Expression &rhs;
    BinaryOperator(Expression &lhs, int op, Expression &rhs) : lhs(lhs), op(op), rhs(rhs) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class UnaryOperator : public Expression
{
public:
    int op;
    Expression &exp;
    UnaryOperator(Expression &exp, int op) : exp(exp), op(op) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class InversionOperator : public Expression
{
public:
    int op;
    Identifier &rhs;
    InversionOperator(int op, Identifier &rhs) : op(op), rhs(rhs) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class Assignment : public Expression
{
public:
    Identifier &lhs;
    Expression &rhs;
    Assignment(Identifier &lhs, Expression &rhs) : lhs(lhs), rhs(rhs) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class Block : public Expression
{
public:
    StatementList statements;
    Block() {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class ExpressionStatement : public Statement
{
public:
    Expression &expression;
    ExpressionStatement(Expression &expression) : expression(expression) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class VariableDeclaration : public Statement
{
public:
    Identifier &type;
    Identifier &id;
    Expression *assignmentExpression;
    VariableDeclaration(Identifier &type, Identifier &id) : type(type), id(id) {}
    VariableDeclaration(Identifier &type, Identifier &id, Expression *assignmentExpression) : type(type), id(id), assignmentExpression(assignmentExpression) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class FunctionDeclaration : public Statement
{
public:
    const Identifier &type;
    const Identifier &id;
    VariableList arguments;
    Block &block;
    FunctionDeclaration(const Identifier &type, const Identifier &id, const VariableList &arguments, Block &block) : type(type), id(id), arguments(arguments), block(block) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class Conditional : public Statement
{
public:
    Expression *comparison;
    Block *onTrue;
    Block *onFalse;

    Conditional(Expression *comparison, Block *onTrue) : comparison(comparison), onTrue(onTrue) {}
    Conditional(Expression *comparison, Block *onTrue, Block *onFalse) : comparison(comparison), onTrue(onTrue), onFalse(onFalse) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};

class ReturnStatement : public Statement
{
public:
    Expression &returnExpression;
    ReturnStatement(Expression &returnExpression) : returnExpression(returnExpression) {}
    virtual llvm::Value *generateCode(GeneratorContext &context);
};