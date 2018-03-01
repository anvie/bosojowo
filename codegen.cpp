#include "node.h"
#include "codegen.h"
#include "parser.hpp"
#include "stringutil.h"

using namespace std;

Value* ensureValue(Value* valOrPtr, BasicBlock* block){
    if (valOrPtr->getType()->isPointerTy()) {
        return new LoadInst(valOrPtr, "", false, block);
    }
    return valOrPtr;
}

CodeGenContext::CodeGenContext(){

    module = new Module("main", getGlobalContext());

}


/* Compile the AST into a module */
void CodeGenContext::generateCode(NBlock& root)
{
    std::cout << "Generating code...n";

    llvm::LLVMContext& context = llvm::getGlobalContext();

    /* Create the top level interpreter function to call as entry */

    vector<Type*> _argTypes;
    ArrayRef<Type*> argTypes(_argTypes);

    module->setTargetTriple("x86_64-apple-darwin14.5.0");


    std::vector<Type*> putsArgs;
    putsArgs.push_back(Type::getInt8Ty(context)->getPointerTo());
    ArrayRef<Type*> argsRef(putsArgs);

    FunctionType* putsType = FunctionType::get(Type::getInt32Ty(context), argsRef, false);
    Constant* putsFunc = module->getOrInsertFunction("puts", putsType);

    {
        std::vector<Type*> args;
        args.push_back(Type::getInt8Ty(context)->getPointerTo());
        ArrayRef<Type*> argsRef(args);

        FunctionType* fTy = FunctionType::get(Type::getInt32Ty(context), argsRef, true);
        Constant* printfFunc = module->getOrInsertFunction("printf", fTy);

        FunctionType* fTy2 = FunctionType::get(Type::getVoidTy(context), argsRef, false);
        Function* tampilFunc = Function::Create(fTy2, GlobalValue::PrivateLinkage, "weruhi", module);

        BasicBlock *bblock = BasicBlock::Create(context, "entry", tampilFunc, 0);

        pushBlock(bblock);

        std::vector<Value*> args2;
        Function::arg_iterator it;
        for (it = tampilFunc->arg_begin(); it != tampilFunc->arg_end(); it++) {
            (*it).setName("text");
            args2.push_back(it);
        }

        ArrayRef<Value*> args2Arr(args2);
        CallInst *call = CallInst::Create(printfFunc, args2Arr, "", bblock);

        std::vector<Value*> args3;

        IRBuilder<> builder(bblock);
        args3.push_back(builder.CreateGlobalStringPtr("\n", "new_line"));

        ArrayRef<Value*> args3Arr(args3);

        CallInst::Create(printfFunc, args3Arr, "", bblock);

        ReturnInst::Create(context, bblock);
        popBlock();
    }

    // build main function
    FunctionType *ftype = FunctionType::get(Type::getVoidTy(context), argTypes, false);
    mainFunction = Function::Create(ftype, GlobalValue::ExternalLinkage, "main", module);
    mainFunction->setVisibility(GlobalValue::VisibilityTypes::DefaultVisibility);
    BasicBlock *bblock = BasicBlock::Create(context, "entry", mainFunction, 0);

    /* Push a new variable/block context */
    pushBlock(bblock);
    root.codeGen(*this); /* emit bytecode for the toplevel block */
    ReturnInst::Create(context, bblock);
    popBlock();

    /* Print the bytecode in a human-readable format
     to see if our program compiled properly
     */
    std::cout << "Code is generated.n";
    //  PassManager<Module> pm;

    legacy::PassManager pm;

    //    const string banner = "-banner-";
    //    llvm::raw_ostream* os = &outs();

    pm.add(llvm::createPrintModulePass(outs()));

    pm.run(*module);

}

/* Executes the AST by running the main function */
GenericValue CodeGenContext::runCode() {

    InitializeNativeTarget();

    //     LLVMContext Context;

    // Create some module to put our function into it.
    std::unique_ptr<Module> Owner = llvm::make_unique<Module>("test", getGlobalContext());
    Module *M = Owner.get();

    ExecutionEngine* EE = EngineBuilder(std::move(Owner)).create();

    outs() << "We just constructed this LLVM module:\n\n" << *M;
    outs() << "\n\nRunning foo: ";
    outs().flush();

    // Call the `foo' function with no arguments:
    std::vector<GenericValue> noargs;
    GenericValue gv = EE->runFunction(mainFunction, noargs);

    return gv;

}

/* Returns an LLVM type based on the identifier */
static Type *typeOf(const NIdentifier& type)
{
    if (type.name.compare("int") == 0) {
        return Type::getInt64Ty(getGlobalContext());
    }
    else if (type.name.compare("double") == 0) {
        return Type::getDoubleTy(getGlobalContext());
    }else if (type.name.compare("str") == 0){
        return Type::getInt8Ty(getGlobalContext())->getPointerTo();
    }
    return Type::getVoidTy(getGlobalContext());
}

/* -- Code Generation -- */

Value* NInteger::codeGen(CodeGenContext& context)
{
    std::cout << "Creating integer: " << value << std::endl;
    return ConstantInt::get(Type::getInt64Ty(getGlobalContext()), value, true);
}

Value* NDouble::codeGen(CodeGenContext& context)
{
    std::cout << "Creating double: " << value << std::endl;
    return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), value);
}

//Value* NVoidExpression::codeGen(CodeGenContext &context)
//{
//    return nullptr;
//}

Value* NReturn::codeGen(CodeGenContext& context){
    LLVMContext& ctx = getGlobalContext();
    return ReturnInst::Create(ctx, lhs->codeGen(context), context.currentBlock());
}

Value* NStr::codeGen(CodeGenContext &context){
    std::cout << "Creating str ref: " << text << std::endl;

    IRBuilder<> builder(context.currentBlock());
    return builder.CreateGlobalStringPtr(str_def(text), "str");

}

Value* NMethodCall::codeGen(CodeGenContext& context)
{
    Function *function = context.module->getFunction(id.name.c_str());
    if (function == NULL) {
        std::cerr << "no such function " << id.name << std::endl;
    }
    std::vector<Value*> args;
    ExpressionList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {

        NExpression *exp = (*it);

        if (context.localVarDecs().find(exp->key()) != context.localVarDecs().end()){
//            args.push_back(context.localVarDecs()[exp->key()]->codeGen(context));

            Value* lVal = ensureValue((*exp).codeGen(context), context.currentBlock());
            args.push_back(lVal);

        }else{
            args.push_back((*exp).codeGen(context));
        }

//        std::cout << "exp->kind(): " << exp->key() << std::endl;


    }
    // CallInst *call = CallInst::Create(function, args.begin(), args.end(), "", context.currentBlock());
    ArrayRef<Value*> args2(args);
    CallInst *call = CallInst::Create(function, args2, "", context.currentBlock());
    std::cout << "Creating method call: " << id.name << std::endl;
    return call;

    //    std::cout << "Creating double: " << value << std::endl;
    //    return ConstantFP::get(Type::getDoubleTy(getGlobalContext()), 0.123);
}



Value* NBinaryOperator::codeGen(CodeGenContext& context)
{
    std::cout << "Creating binary operation " << op << std::endl;
    Instruction::BinaryOps instr;

    IRBuilder<> builder(context.currentBlock());

    Value* lval = ensureValue(lhs.codeGen(context), context.currentBlock());
    Value* rval = ensureValue(rhs.codeGen(context), context.currentBlock());

    switch (op) {
        case TPLUS:
            instr = Instruction::FAdd;

            using namespace std;

            //            Value* _code = rhs.codeGen(context);

//            cout << "rhs.kind(): " << rhs.kind() << ", ptr?: ";
//            cout << (rhs.codeGen(context)->getType()->isPointerTy() ? "yes" : "no") << endl;

            goto math;
        case TMINUS: instr = Instruction::FSub; goto math;
        case TMUL:
            instr = Instruction::FMul;
//            std::cout << "lhs.kind(): " << lhs.kind() << std::endl;
////            context.locals().find(lhs.key())
//            if (lhs.kind().compare("double") == 0 || rhs.kind().compare("double") == 0) {
//                instr = Instruction::FMul;
//            }
            //goto math;

            using namespace std;

//            Value* _code = rhs.codeGen(context);

//            cout << "rhs.kind(): " << rhs.kind() << ", ptr?: ";
//            cout << (rhs.codeGen(context)->getType()->isPointerTy() ? "yes" : "no") << endl;

            goto math;

        case TDIV:
            instr = Instruction::SDiv; goto math;

        case TCLT: // < ( less than )
            return builder.CreateFCmp(CmpInst::Predicate::FCMP_OLT, lval, rval);


        case TCGT: // > ( greater than )
            return builder.CreateFCmp(CmpInst::Predicate::FCMP_OGT, lval, rval);

            /* TODO comparison */
    }

    return NULL;
math:
//    return BinaryOperator::Create(instr, lhs.codeGen(context),
//                                  rhs.codeGen(context), "", context.currentBlock());

    return BinaryOperator::Create(instr, lval, rval, "", context.currentBlock());


//math2:
//    rhs.codeGen(context)->print(outs());
//    LoadInst* _load = new LoadInst(rhs.codeGen(context), "", false, context.currentBlock());
//    return BinaryOperator::Create(instr, lhs.codeGen(context),
//                           _load, "", context.currentBlock());

}

Value* NAssignment::codeGen(CodeGenContext& context)
{
    std::cout << "Creating assignment for " << lhs.name << std::endl;
    if (context.locals().find(lhs.name) == context.locals().end()) {
        std::cerr << "undeclared variable " << lhs.name << std::endl;
        return NULL;
    }
    return new StoreInst(rhs.codeGen(context), context.locals()[lhs.name], true, context.currentBlock());
}

Value* NBlock::codeGen(CodeGenContext& context)
{
    StatementList::const_iterator it;
    Value *last = NULL;
    for (it = statements.begin(); it != statements.end(); it++) {
        //std::cout << "Generating code for " << typeid(**it).name() << std::endl;
        last = (**it).codeGen(context);
    }
    std::cout << "Creating block" << std::endl;
    return last;
}

Value* NConditionalBlock::codeGen(CodeGenContext& context)
{

    Value *condCode = cond.codeGen(context);

    std::cout << "cond.kind(): " << cond.kind() << std::endl;

    if (!condCode) {
        return nullptr;
    }


    IRBuilder<> builder(context.currentBlock());

    Function *theFunction = context.currentBlock()->getParent();

//    condCode->setName("ifcond");
    BasicBlock *thenBB = BasicBlock::Create(getGlobalContext(), "then", theFunction);
    BasicBlock *elseBB = BasicBlock::Create(getGlobalContext(), "else");
//    BasicBlock *mergeBB = BasicBlock::Create(getGlobalContext(), "ifcont");

    BranchInst::Create(thenBB, elseBB, condCode, context.currentBlock());
//    builder.CreateCondBr(condCode, thenBB, elseBB);


    // get locals
    std::map<std::string, Value*>& parentLocals = context.locals();

    context.pushBlock(thenBB);
    context.copyLocals(parentLocals); // copy local dari parent-nya ke current block

    builder.SetInsertPoint(thenBB);

    Value *thenV = thenStmt->codeGen(context);
    if (!thenV)
        return nullptr;

    context.popBlock();

    theFunction->getBasicBlockList().push_back(elseBB);
    context.pushBlock(elseBB);
    context.copyLocals(parentLocals);

    builder.SetInsertPoint(elseBB);
    Value* elseV = elseStmt->codeGen(context);
    if (!elseV) {
        return nullptr;
    }

    context.popBlock();


    return nullptr;

}


Value* NLoop::codeGen(CodeGenContext& context)
{

    Value *fromCode = exprFrom.codeGen(context);

    std::cout << "exprFrom.kind(): " << exprFrom.kind() << std::endl;

    if (!fromCode) {
        return nullptr;
    }


    IRBuilder<> builder(context.currentBlock());

    BasicBlock* preHeaderBB = builder.GetInsertBlock();

    Function *theFunction = context.currentBlock()->getParent();

    BasicBlock *loopBlock = BasicBlock::Create(getGlobalContext(), "loop", theFunction);

    Type* doubleTy = Type::getDoubleTy(getGlobalContext());

    // get locals
    std::map<std::string, Value*>& parentLocals = context.locals();

    context.pushBlock(loopBlock);
    context.copyLocals(parentLocals); // copy local dari parent-nya ke current block

    PHINode* Variable = PHINode::Create(doubleTy, 2, "i", loopBlock);
    Variable->addIncoming(fromCode, preHeaderBB);
    context.locals()["i"] = Variable;

    this->block->codeGen(context);

    builder.CreateBr(loopBlock);
    builder.SetInsertPoint(loopBlock);


//    AllocaInst *alloc = new AllocaInst(doubleTy, "i", context.currentBlock());
//    context.locals()["i"] = alloc;


    Value* exprUntilCode = exprUntil.codeGen(context);

    Value* nextVar = builder.CreateFAdd(Variable, ConstantFP::get(getGlobalContext(), APFloat(1.0)), "nextvar");

    BasicBlock* loopEndBB = builder.GetInsertBlock();
    BasicBlock* afterBB = BasicBlock::Create(getGlobalContext(), "afterloop", theFunction);


    Value* compareV = builder.CreateFCmpOLE(nextVar, exprUntilCode, "loopcond");

    builder.CreateCondBr(compareV, loopBlock, afterBB);

    Variable->addIncoming(nextVar, loopEndBB);

    context.popBlock();

    // void return
    context.pushBlock(afterBB);
    builder.SetInsertPoint(afterBB);
    Value* returnInst = ReturnInst::Create(getGlobalContext(), nullptr, context.currentBlock());
    context.popBlock();

    return nullptr;

}

Value* NExpressionStatement::codeGen(CodeGenContext& context)
{
    std::cout << "Generating code for " << typeid(expression).name() << std::endl;
    return expression.codeGen(context);
}


Value* NIdentifier::codeGen(CodeGenContext& context)
{
    std::cout << "Creating identifier reference: " << name << std::endl;
    if (context.locals().find(name) == context.locals().end()) {
        std::cerr << "undeclared variable " << name << std::endl;
        return NULL;
    }
    //    if (!context.locals()[name]->isUsedInBasicBlock(context.currentBlock())){
    return context.locals()[name];
    //    }else{
    //        return new LoadInst(context.locals()[name], "", false, context.currentBlock());
    //    }
}


Value* NVariableDeclaration::codeGen(CodeGenContext& context)
{
    std::cout << "Creating variable declaration " << type.name << " " << id.name << std::endl;

    if (assignmentExpr != NULL){
        AllocaInst *alloc = new AllocaInst(typeOf(type), id.name.c_str(), context.currentBlock());
        context.locals()[id.name] = alloc;
        if (assignmentExpr != NULL) {
            NAssignment assn(id, *assignmentExpr);
            assn.codeGen(context);
        }

        // masukkan ke local scope untuk keperluan ref var
        context.localVarDecs()[id.name] = this;

        return alloc;
    }else{
//        context.locals()[id.name] = id;
        return id.codeGen(context);
    }


}


Value* NFunctionDeclaration::codeGen(CodeGenContext& context)
{
    vector<Type*> _argTypes;
    VariableList::const_iterator it;
    for (it = arguments.begin(); it != arguments.end(); it++) {
        _argTypes.push_back(typeOf((**it).type));
    }

    ArrayRef<Type*> argTypes(_argTypes);

    FunctionType *ftype = FunctionType::get(Type::getVoidTy(getGlobalContext()), argTypes, false);
    if (this->type != nullptr) {
        ftype = FunctionType::get(typeOf(*type), argTypes, false);
    }
    Function *function = Function::Create(ftype, GlobalValue::InternalLinkage, id.name.c_str(), context.module);

    BasicBlock *bblock = BasicBlock::Create(getGlobalContext(), "entry", function, 0);

    context.pushBlock(bblock);

    // setting arguments-name
    // masukkan setiap var args ke context.locals untuk diproses kemudian oleh block.
    {
        VariableList::const_iterator argIt = arguments.begin();
        Function::arg_iterator it = function->arg_begin();

        for (it = function->arg_begin(); it != function->arg_end(); it++) {
            string& name = (**argIt).id.name;

            //std::cout << "name: " << name << std::endl;

            context.locals()[name] = it;
            context.localVarDecs()[name] = *argIt;

            (*it).setName(name);

            argIt++;
        }

    }

    // periksa apakah di dalam block sudah ada return statement
    // apabila belum ada maka perlu menambahkan void return.
    StatementList::iterator s_it = block.statements.begin();
    vector<NStatement*> nStatements;

    bool hasReturn = false;
    for (s_it = block.statements.begin(); s_it != block.statements.end(); s_it++){
        std::cout << "s_it: " << (*s_it)->kind() << std::endl;
        if ((*s_it)->kind() == "Return") {
            hasReturn = true;
            break;
        }else if ((*s_it)->kind() == "Block") {
            StatementList statements = static_cast<NBlock*>((static_cast<NExpression*>(*s_it)))->statements;

        }
    }

    // hanya apabila tidak memiliki return dan type func-nya adalah void.
    if (!hasReturn && ftype->getReturnType()->isVoidTy()){
        // apabila tidak memiliki return
        // buatkan void return.
        //if (block.statements.size() == 0){
            block.statements.push_back(new NReturn());
        //}
    }

    block.codeGen(context);

    context.popBlock();
    std::cout << "Creating function: " << id.name << std::endl;
    return function;
}
