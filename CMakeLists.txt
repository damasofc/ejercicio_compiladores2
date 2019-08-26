cmake_minimum_required (VERSION 2.6)
project (expr)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14")

add_custom_command(
  OUTPUT expr_ast.cpp expr_ast.h 
  COMMAND treecc -o expr_ast.cpp -h expr_ast.h ${PROJECT_SOURCE_DIR}/ejercisio.tc
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/ejercisio.tc
)

add_custom_command(
  OUTPUT expr_parser.cpp
  COMMAND bison --defines=Tokens.h ${PROJECT_SOURCE_DIR}/parser.y -o expr_parser.cpp
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/parser.y
)

# Run flex to generate the lexer
add_custom_command(
  OUTPUT expr_lexer.cpp
  COMMAND flex -o expr_lexer.cpp ${PROJECT_SOURCE_DIR}/ejerc.l
  MAIN_DEPENDENCY ${PROJECT_SOURCE_DIR}/ejerc.l
)

include_directories(${PROJECT_SOURCE_DIR})
 
add_executable(${PROJECT_NAME} 
                  ${PROJECT_SOURCE_DIR}/build/expr_lexer.cpp 
                  ${PROJECT_SOURCE_DIR}/build/expr_ast.cpp
                  ${PROJECT_SOURCE_DIR}/build/expr_parser.cpp )



