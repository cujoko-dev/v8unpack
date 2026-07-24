if(NOT DEFINED PROGRAM OR NOT DEFINED FIXTURE OR NOT DEFINED WORK_DIR)
    message(FATAL_ERROR "PROGRAM, FIXTURE and WORK_DIR are required")
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
        message(FATAL_ERROR "Round-trip tree entries differ")
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
file(MAKE_DIRECTORY "${WORK_DIR}")

run_checked("Fixture PARSE" "${PROGRAM}" -PARSE
    "${FIXTURE}" "${WORK_DIR}/parsed")
run_checked("Fixture BUILD" "${PROGRAM}" -BUILD
    "${WORK_DIR}/parsed" "${WORK_DIR}/rebuilt.epf")
run_checked("Fixture reparse" "${PROGRAM}" -PARSE
    "${WORK_DIR}/rebuilt.epf" "${WORK_DIR}/reparsed")

file(SHA256 "${WORK_DIR}/rebuilt.epf" rebuilt_sha256)
string(TOLOWER "${EXPECTED_SHA256}" expected_sha256)
if(DEFINED EXPECTED_SHA256 AND NOT rebuilt_sha256 STREQUAL expected_sha256)
    message(FATAL_ERROR
        "Fixture SHA-256 mismatch: expected ${EXPECTED_SHA256}, got ${rebuilt_sha256}")
endif()

compare_trees("${WORK_DIR}/parsed" "${WORK_DIR}/reparsed")
