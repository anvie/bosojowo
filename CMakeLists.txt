CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

PROJECT(bosojowo)

FIND_PACKAGE(LLVM)

message(STATUS aa ${LLVM_INCLUDE_DIRS} - ${LLVM_CXXFLAGS} - ${LLVM_LDFLAGS} - ${LLVM_LIBRARY_DIRS})

ADD_CUSTOM_TARGET(tokens.cpp
 COMMAND "lex" -o tokens.cpp tokens.l
 )

ADD_CUSTOM_TARGET(parser.cpp
 COMMAND "bison" -t -d -o parser.cpp parser.y
 DEPENDS tokens.cpp
 )

SET(SOURCES stringutil.cpp tokens.cpp parser.cpp codegen.cpp test.cpp)

INCLUDE_DIRECTORIES(BEFORE /usr/local/include)
INCLUDE_DIRECTORIES(AFTER ${LLVM_INCLUDE_DIRS})

FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

FUNCTION(ADD_EXTENSION var extension)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${f}.${extension}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(ADD_EXTENSION)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY, ${CMAKE_BINARY_DIR}/bin)

#llvm_map_components_to_libnames(REQ_LLVM_LIBRARIES native)

#PREPEND(REQ_LLVM_LIBRARIES2 ${LLVM_LIBRARY_DIRS} ${REQ_LLVM_LIBRARIES})
#ADD_EXTENSION(REQ_LLVM_LIBRARIES2 "dylib" ${REQ_LLVM_LIBRARIES2})
#STRING(REPLACE ";" " " REQ_LLVM_LIBRARIES2 "${REQ_LLVM_LIBRARIES2}")

#message(STATUS "LIBS: ${LIBS}")
message(STATUS "REQ_LLVM_LIBRARIES2: ${REQ_LLVM_LIBRARIES2}")

ADD_DEFINITIONS(
 -arch x86_64
 -std=c++1y -fmodules -gmodules
 -D__STDC_LIMIT_MACROS
 -D__STDC_CONSTANT_MACROS
 )

SET(CMAKE_EXE_LINKER_FLAGS
 "${CMAKE_EXE_LINKER_FLAGS} -L/usr/lib -lz -ll -lc++ -lncurses -L${LLVM_LIBRARY_DIRS} -lLLVMLTO -lLLVMObjCARCOpts -lLLVMLinker -lLLVMBitWriter -lLLVMIRReader -lLLVMAsmParser -lLLVMXCoreDisassembler -lLLVMXCoreCodeGen -lLLVMXCoreDesc -lLLVMXCoreInfo -lLLVMXCoreAsmPrinter -lLLVMSystemZDisassembler -lLLVMSystemZCodeGen -lLLVMSystemZAsmParser -lLLVMSystemZDesc -lLLVMSystemZInfo -lLLVMSystemZAsmPrinter -lLLVMSparcDisassembler -lLLVMSparcCodeGen -lLLVMSparcAsmParser -lLLVMSparcDesc -lLLVMSparcInfo -lLLVMSparcAsmPrinter -lLLVMR600CodeGen -lLLVMipo -lLLVMVectorize -lLLVMR600AsmParser -lLLVMR600Desc -lLLVMR600Info -lLLVMR600AsmPrinter -lLLVMPowerPCDisassembler -lLLVMPowerPCCodeGen -lLLVMPowerPCAsmParser -lLLVMPowerPCDesc -lLLVMPowerPCInfo -lLLVMPowerPCAsmPrinter -lLLVMNVPTXCodeGen -lLLVMNVPTXDesc -lLLVMNVPTXInfo -lLLVMNVPTXAsmPrinter -lLLVMMSP430CodeGen -lLLVMMSP430Desc -lLLVMMSP430Info -lLLVMMSP430AsmPrinter -lLLVMMipsDisassembler -lLLVMMipsCodeGen -lLLVMMipsAsmParser -lLLVMMipsDesc -lLLVMMipsInfo -lLLVMMipsAsmPrinter -lLLVMHexagonDisassembler -lLLVMHexagonCodeGen -lLLVMHexagonDesc -lLLVMHexagonInfo -lLLVMCppBackendCodeGen -lLLVMCppBackendInfo -lLLVMARMDisassembler -lLLVMARMCodeGen -lLLVMARMAsmParser -lLLVMARMDesc -lLLVMARMInfo -lLLVMARMAsmPrinter -lLLVMAArch64Disassembler -lLLVMAArch64CodeGen -lLLVMAArch64AsmParser -lLLVMAArch64Desc -lLLVMAArch64Info -lLLVMAArch64AsmPrinter -lLLVMAArch64Utils -lLLVMTableGen -lLLVMDebugInfo -lLLVMOption -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMX86Desc -lLLVMMCDisassembler -lLLVMX86Info -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMMCJIT -lLLVMLineEditor -lLLVMInstrumentation -lLLVMInterpreter -lLLVMExecutionEngine -lLLVMRuntimeDyld -lLLVMCodeGen -lLLVMScalarOpts -lLLVMProfileData -lLLVMObject -lLLVMMCParser -lLLVMBitReader -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMTarget -lLLVMMC -lLLVMCore -lLLVMSupport"
 )

ADD_EXECUTABLE(bosojowo ${SOURCES})
ADD_DEPENDENCIES(bosojowo tokens.cpp parser.cpp)

SET_TARGET_PROPERTIES(bosojowo 
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

#TARGET_LINK_LIBRARIES(bosojowo ${REQ_LLVM_LIBRARIES2})
