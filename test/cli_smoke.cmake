if(NOT DEFINED PROGRAM OR NOT DEFINED WORK_DIR)
    message(FATAL_ERROR "PROGRAM and WORK_DIR are required")
endif()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")
set(INVALID_FILE "${WORK_DIR}/invalid.cf")
set(MANIFEST_FILE "${WORK_DIR}/manifest.json")
file(WRITE "${INVALID_FILE}" "not a container")

execute_process(
    COMMAND "${PROGRAM}" --version
    RESULT_VARIABLE version_result
    OUTPUT_VARIABLE version_output
)
if(NOT version_result EQUAL 0 OR NOT version_output MATCHES "${EXPECTED_VERSION}")
    message(FATAL_ERROR "Version command failed: ${version_output}")
endif()

execute_process(
    COMMAND "${PROGRAM}" check "${INVALID_FILE}" --json
    RESULT_VARIABLE check_result
    OUTPUT_VARIABLE check_output
)
if(NOT check_result EQUAL 2 OR NOT check_output MATCHES "\\\"valid\\\":false")
    message(FATAL_ERROR "Invalid-container check did not fail cleanly: ${check_output}")
endif()

execute_process(
    COMMAND "${PROGRAM}" manifest "${INVALID_FILE}" "${MANIFEST_FILE}" --quiet
    RESULT_VARIABLE manifest_result
)
if(NOT manifest_result EQUAL 2 OR NOT EXISTS "${MANIFEST_FILE}")
    message(FATAL_ERROR "Manifest command did not produce diagnostics for an invalid file")
endif()

execute_process(
    COMMAND "${PROGRAM}" manifest "${INVALID_FILE}" "${MANIFEST_FILE}" --quiet
    RESULT_VARIABLE overwrite_result
)
if(NOT overwrite_result EQUAL 1)
    message(FATAL_ERROR "Existing output was overwritten without --force")
endif()

execute_process(
    COMMAND "${PROGRAM}" manifest "${INVALID_FILE}" "${MANIFEST_FILE}" --quiet --force
    RESULT_VARIABLE force_result
)
if(NOT force_result EQUAL 2)
    message(FATAL_ERROR "--force did not permit replacing an existing manifest")
endif()
