# this doesn't have to be a variable, this just makes it easier to change later
set(TARGET_NAME cc)

# add all source files here
list(APPEND COMPILER_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/analysis.c
  ${CMAKE_CURRENT_LIST_DIR}/codegen.c
  ${CMAKE_CURRENT_LIST_DIR}/lexer.c
  ${CMAKE_CURRENT_LIST_DIR}/main.c
  ${CMAKE_CURRENT_LIST_DIR}/misc.c
  ${CMAKE_CURRENT_LIST_DIR}/parser.c
  ${CMAKE_CURRENT_LIST_DIR}/builtins.c
  ${CMAKE_CURRENT_LIST_DIR}/type.c
  ${CMAKE_CURRENT_LIST_DIR}/ast/if_stmt.c
  ${CMAKE_CURRENT_LIST_DIR}/ast/return.c
  ${CMAKE_CURRENT_LIST_DIR}/ast/variables.c
  ${CMAKE_CURRENT_LIST_DIR}/ast/literals.c
  ${CMAKE_CURRENT_LIST_DIR}/ast/fn_call.c
  ${CMAKE_CURRENT_LIST_DIR}/ast/casts.c
  ${CMAKE_CURRENT_LIST_DIR}/vec.c)

# tell cmake about our executable
add_executable(${TARGET_NAME} ${COMPILER_SOURCES})

# add more include directories here
target_include_directories(${TARGET_NAME} PUBLIC
  "${CMAKE_SOURCE_DIR}/include"
  ${LLVM_INCDIR})

find_library(LIBGC gc)

# add more libraries here if necessary
target_link_libraries(${TARGET_NAME}
  ${LIBGC}
  ${LLVM_LIBS})

#target_link_libraries(${TARGET_NAME} debug
#  -fsanitize=address -fsanitize=undefined
#)
target_compile_options(${TARGET_NAME} PRIVATE
  # use -g for debug, -O3 for release
  $<$<CONFIG:Debug>:
  -g #-fsanitize=address -fsanitize=undefined
  >

  $<$<CONFIG:Release>:
  -O3
  >
  )

