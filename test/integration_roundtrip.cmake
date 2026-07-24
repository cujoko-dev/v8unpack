if(NOT DEFINED PROGRAM OR NOT DEFINED WORK_DIR)
    message(FATAL_ERROR "PROGRAM and WORK_DIR are required")
endif()

function(run_checked description)
    execute_process(
        COMMAND ${ARGN}
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        ERROR_VARIABLE error
    )
    if(NOT result EQUAL 0)
        message(FATAL_ERROR
            "${description} failed with exit code ${result}\n${output}\n${error}")
    endif()
endfunction()

function(compare_trees expected actual)
    file(GLOB_RECURSE expected_files RELATIVE "${expected}" "${expected}/*")
    file(GLOB_RECURSE actual_files RELATIVE "${actual}" "${actual}/*")
    list(SORT expected_files)
    list(SORT actual_files)
    if(NOT expected_files STREQUAL actual_files)
        message(FATAL_ERROR
            "Tree entries differ\nexpected: ${expected_files}\nactual: ${actual_files}")
    endif()
    foreach(relative_path IN LISTS expected_files)
        if(NOT IS_DIRECTORY "${expected}/${relative_path}")
            run_checked(
                "Comparing ${relative_path}"
                "${CMAKE_COMMAND}" -E compare_files
                "${expected}/${relative_path}" "${actual}/${relative_path}")
        endif()
    endforeach()
endfunction()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}/input/nested/deeper")
file(WRITE "${WORK_DIR}/input/root.txt" "v8unpack deterministic integration test\n")
file(WRITE "${WORK_DIR}/input/nested/data.bin" "0123456789abcdef\n")
file(WRITE "${WORK_DIR}/input/nested/deeper/empty.txt" "")

run_checked("BUILD" "${PROGRAM}" -BUILD
    "${WORK_DIR}/input" "${WORK_DIR}/generated.cf")
run_checked("PARSE" "${PROGRAM}" -PARSE
    "${WORK_DIR}/generated.cf" "${WORK_DIR}/parsed")
compare_trees("${WORK_DIR}/input" "${WORK_DIR}/parsed")

run_checked("Rebuild" "${PROGRAM}" -BUILD
    "${WORK_DIR}/parsed" "${WORK_DIR}/rebuilt.cf")
run_checked("Reparse" "${PROGRAM}" -PARSE
    "${WORK_DIR}/rebuilt.cf" "${WORK_DIR}/reparsed")
compare_trees("${WORK_DIR}/input" "${WORK_DIR}/reparsed")
