#include <iostream>
#include <fstream>
#include "codegen.h"
#include "node.h"



#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_os_ostream.h"

//using namespace llvm;



extern NBlock* programBlock;
extern int yydebug;
extern int yyparse();

int main(int argc, char **argv)
{
    
//    llvm::LLVMContext& context = llvm::getGlobalContext();
//    llvm::Module* module = new llvm::Module("top", context);
    
    yydebug=1;
    
    if (argc < 2){
        std::cout << "Usage: exe [input-file]" << std::endl;
        return 2;
    }
    
  yyparse();
    
    std::cout << "hello" << std::endl;
    
  std::cout << programBlock << std::endl;
    
//    module->dump();
    
    CodeGenContext context;

    context.generateCode(*programBlock);
//    context.module->dump();
//    context.runCode();
    
    {
        std::string filePath = std::string(argv[1]);
        std::ofstream outFile(filePath, std::ios::binary);
        raw_os_ostream outFileOsStream(outFile);
        context.module->print(outFileOsStream, nullptr);
        std::cout << std::endl;
        std::cout << "out: " << filePath << std::endl;
    }
  
  return 0;
}
