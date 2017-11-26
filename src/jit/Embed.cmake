cmake_minimum_required(VERSION 3.7)

enable_language(ASM)

function(EMBED_FILES ASM_FILES)
    cmake_parse_arguments(llvm "" "EXPORT_MACRO" "" ${ARGN})
    set(INPUT_FILES "${llvm_UNPARSED_ARGUMENTS}")

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")

    set(${ASM_FILES})

    foreach(FILE ${INPUT_FILES})
        get_filename_component(FILE_EXTENSION ${FILE} EXT)
        get_filename_component(CPP_FILE ${FILE} ABSOLUTE)
        get_filename_component(FILENAME ${FILE} NAME_WE)

        if(NOT ${FILE_EXTENSION} MATCHES ".cpp")
            continue()
        endif()

        embed_file(${FILENAME}_cpp ${CPP_FILE} ${ASM_FILES})

        set(FLAGS -std=c++1z ${CMAKE_CXX_FLAGS})
        set(PREPROC_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated/${FILENAME}.cpp")
        add_custom_command(
                OUTPUT ${PREPROC_FILE}
                COMMAND clang++-5.0 ${FLAGS} -E -S -o ${PREPROC_FILE} ${CPP_FILE}
                DEPENDS ${CPP_FILE} ${XX_FILE})
        embed_file(${FILENAME}_processed_cpp ${PREPROC_FILE} ${ASM_FILES})
    endforeach()

    set_source_files_properties(${ASM_FILES} PROPERTIES GENERATED TRUE)
    set(${ASM_FILES} ${${ASM_FILES}} PARENT_SCOPE)
endfunction()

function(EMBED_FILE SYMBOL FILE ASM_FILES)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")

    set(ASM_FILE "${CMAKE_CURRENT_BINARY_DIR}/generated/${SYMBOL}.s")

    add_custom_command(
            OUTPUT ${ASM_FILE}
            COMMAND echo \".global ${SYMBOL}\\n.global ${SYMBOL}_size\\n.global _${SYMBOL}\\n.global _${SYMBOL}_size\\n${SYMBOL}:\\n_${SYMBOL}:\\n.incbin \\"${FILE}\\"\\n1:\\n${SYMBOL}_size:\\n_${SYMBOL}_size:\\n.8byte 1b - ${SYMBOL}\\n\" > ${ASM_FILE}
            DEPENDS ${FILE})

    list(APPEND ${ASM_FILES} ${ASM_FILE})
    set(${ASM_FILES} "${${ASM_FILES}}" PARENT_SCOPE)
endfunction()
