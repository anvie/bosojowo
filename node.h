#include <iostream>
#include <vector>
#include <llvm/IR/Value.h>

class CodeGenContext;
class NStatement;
class NExpression;
class NVariableDeclaration;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
typedef std::vector<NVariableDeclaration*> VariableList;

class Node {
public:
    virtual ~Node() {}
    virtual llvm::Value* codeGen(CodeGenContext& context){ return nullptr; };
    virtual std::string kind(){ return "Node"; };
};

class NExpression : public Node {
public:
    virtual std::string key(){ return ""; };
    virtual std::string kind(){ return "Expr"; };
};

class NVoidExpression : public NExpression {
public:
    NVoidExpression(){}
    virtual llvm::Value* codeGen(CodeGenContext& context){ return nullptr; };
    virtual std::string kind(){ return "VoidExp"; };
};

class NStatement : public NExpression {
public:
    virtual std::string kind(){ return "Statement"; };
};

class NInteger : public NExpression {
public:
    long long value;
    NInteger(long long value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    virtual std::string kind(){ return "int"; };
};

class NDouble : public NExpression {
public:
    double value;
    NDouble(double value) : value(value) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    virtual std::string kind(){ return "double"; };
};

class NIdentifier : public NExpression {
public:
    std::string name;
//    bool isArg;
    NIdentifier(const std::string& name) : name(name) { }
//    NIdentifier(const std::string& name, bool isArg) : name(name), isArg(isArg) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    virtual std::string kind(){ return "ident=" + name; };
    virtual std::string key(){ return name; };
};

class NReturn : public NStatement {
public:
    NExpression* lhs;
    NReturn(NExpression* lhs) : lhs(lhs) { }
    NReturn(): lhs(new NVoidExpression()) {}
    ~NReturn(){
        if (lhs != nullptr){
            delete lhs;
        }
    }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    virtual std::string kind(){ return "Return"; };
};

class NStr : public NExpression {
public:
    std::string text;
    NStr(const std::string& text) : text(text) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
    virtual std::string kind(){ return "str"; };
};

class NMethodCall : public NExpression {
public:
    const NIdentifier& id;
    ExpressionList arguments;
    NMethodCall(const NIdentifier& id, ExpressionList& arguments) :
        id(id), arguments(arguments) { }
    NMethodCall(const NIdentifier& id) : id(id) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    virtual std::string kind(){ return "MethodCall"; };
};

class NBinaryOperator : public NExpression {
public:
    int op;
    NExpression& lhs;
    NExpression& rhs;
    NBinaryOperator(NExpression& lhs, int op, NExpression& rhs) :
        op(op), lhs(lhs), rhs(rhs) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    virtual std::string kind(){ return "BinaryOperator"; }
};

class NAssignment : public NExpression {
public:
    NIdentifier& lhs;
    NExpression& rhs;
    NAssignment(NIdentifier& lhs, NExpression& rhs) :
        lhs(lhs), rhs(rhs) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NBlock : public NExpression {
public:
    StatementList statements;
    NBlock() { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
    virtual std::string kind(){ return "Block"; }
};

class NConditionalBlock : public NExpression {
public:
    NExpression &cond;
    NBlock *thenStmt, *elseStmt;
    
    NConditionalBlock(NExpression& cond, NBlock* tb, NBlock* eb):
        cond(cond), thenStmt(tb), elseStmt(eb) {}
    virtual llvm::Value* codeGen(CodeGenContext& context);
};


class NLoop : public NExpression {
public:
    NExpression &exprFrom;
    NExpression &exprUntil;
    NBlock* block;
    
    NLoop(NExpression& exprFrom, NExpression& exprUntil, NBlock* block):
        exprFrom(exprFrom), exprUntil(exprUntil), block(block){}
    
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

//
//class NForLoopExp : public NExpression {
//public:
//    NExpression &cond;
//    NBlock *thenStmt, *elseStmt;
//    
//    NForLoopExp(NExpression& cond, NBlock* tb, NBlock* eb):
//    cond(cond), thenStmt(tb), elseStmt(eb) {}
//    virtual llvm::Value* codeGen(CodeGenContext& context);
//};


class NExpressionStatement : public NStatement {
public:
    NExpression& expression;
    NExpressionStatement(NExpression& expression) :
        expression(expression) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NVariableDeclaration : public NStatement {
public:
    const NIdentifier& type;
    NIdentifier& id;
    NExpression *assignmentExpr;
    NVariableDeclaration(const NIdentifier& type, NIdentifier& id) :
        type(type), id(id) { }
    NVariableDeclaration(const NIdentifier& type, NIdentifier& id, NExpression *assignmentExpr) :
        type(type), id(id), assignmentExpr(assignmentExpr) { }
    virtual llvm::Value* codeGen(CodeGenContext& context);
};

class NFunctionDeclaration : public NStatement {
public:
    NIdentifier* type;
    const NIdentifier& id;
    VariableList arguments;
    NBlock& block;
    
    NFunctionDeclaration(NIdentifier* type, const NIdentifier& id,
            const VariableList& arguments, NBlock& block) :
        type(type), id(id), arguments(arguments), block(block) { }

    NFunctionDeclaration(NIdentifier* type, const NIdentifier& id, NBlock& block) :
        type(type), id(id), arguments(VariableList()), block(block) { }

    virtual llvm::Value* codeGen(CodeGenContext& context);
};
