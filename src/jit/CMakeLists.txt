include(./Embed.cmake)

find_package(LLVM 5.0.0 REQUIRED)
message(STATUS ${LLVM_VERSION})
message(STATUS "Found llvm library: inc=${LLVM_INCLUDE_DIR}, lib=${LLVM_LIBRARY_DIR}/libLLVM.so")

include_directories(${LLVM_INCLUDE_DIR})

set(OPERATOR_FILES
    query/operators/aggregate.cpp
    query/operators/aggregate.hpp
    query/operators/get_table.cpp
    query/operators/get_table.hpp
    query/operators/get_table_mock.cpp
    query/operators/get_table_mock.hpp
    query/operators/operator.hpp
    query/operators/print.cpp
    query/operators/print.hpp
    query/operators/source.hpp
    query/operators/table_scan.cpp
    query/operators/table_scan.hpp
)

set(QUERY_FILES
    query/query.cpp
    query/query.hpp
    ${OPERATOR_FILES}
)

set(STAGE_FILES
    jit/stages/allow_inline_stage.cpp
    jit/stages/allow_inline_stage.hpp
    jit/stages/compile_stage.cpp
    jit/stages/compile_stage.hpp
    jit/stages/constant_substitute_stage.cpp
    jit/stages/constant_substitute_stage.hpp
    jit/stages/execute_stage.cpp
    jit/stages/execute_stage.hpp
    jit/stages/opt_stage.cpp
    jit/stages/opt_stage.hpp
    jit/stages/extract_stage.cpp
    jit/stages/extract_stage.hpp
    jit/stages/jit_stage.cpp
    jit/stages/jit_stage.hpp
    jit/stages/manual_inline_stage.cpp
    jit/stages/manual_inline_stage.hpp
    jit/stages/llvm_stage.hpp
    jit/stages/rename_stage.cpp
    jit/stages/rename_stage.hpp
    jit/stages/replace_stage.cpp
    jit/stages/replace_stage.hpp
    jit/stages/stack_args_stage.cpp
    jit/stages/stack_args_stage.hpp
    jit/stages/strip_debug_info_stage.cpp
    jit/stages/strip_debug_info_stage.hpp
    jit/stages/stage.hpp
)

embed_files(OPERATOR_EMBEDDED_FILES ${OPERATOR_FILES})

set(JIT_FILES
    jit/blob_store.cpp
    jit/blob_store.hpp
    jit/jit.cpp
    jit/jit.hpp
    jit/jit_query_executor.cpp
    jit/jit_query_executor.hpp
    jit/pipeline.hpp
    util/error_utils.hpp
    util/file_utils.hpp
    util/fn_utils.hpp
    util/llvm_utils.hpp
    ${OPERATOR_EMBEDDED_FILES}
    ${STAGE_FILES}
)

# Configure jit-playground
add_executable(
    hyriseJitPlayground
    jit_playground.cpp
    ${JIT_FILES}
    ${QUERY_FILES}
)

target_link_libraries(
    hyriseJitPlayground
    #hyrise
    #hyriseBenchmarkLib
    boost_filesystem
    boost_system
    ${LLVM_LIBRARY_DIR}/libLLVM.so
)
