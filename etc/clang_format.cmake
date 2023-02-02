# attempt to find the binary if user did not specify
find_program(CLANG_FORMAT
        NAMES clang-format clang-format-12
        HINTS ${BUSTUB_CLANG_SEARCH_PATH})

if ("${CLANG_FORMAT}" STREQUAL "CLANG_FORMAT_BIN-NOTFOUND")
    message(WARNING "couldn't find clang-format.")
else ()
    message(STATUS "found clang-format at ${CLANG_FORMAT_BIN}")
endif ()

if (DEFINED CLANG_FORMAT)
    file (GLOB_RECURSE ALL_CC_FILES *.cc)
    file (GLOB_RECURSE ALL_HH_FILES *.hh)
    add_custom_target (format ${CLANG_FORMAT} -i ${ALL_CC_FILES} ${ALL_HH_FILES} COMMENT "Formatted all source files.")
else (NOT DEFINED CLANG_FORMAT)
    add_custom_target (format echo "Could not find clang-format. Please install and re-run cmake")
endif (DEFINED CLANG_FORMAT)