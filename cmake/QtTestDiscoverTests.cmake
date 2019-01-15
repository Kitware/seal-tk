# This file is part of SEAL-TK, and is distributed under the OSI-approved BSD
# 3-Clause License. See top-level LICENSE file or
# https://github.com/Kitware/seal-tk/blob/master/LICENSE for details.

execute_process(COMMAND "${TEST_EXECUTABLE}" -datatags
  OUTPUT_VARIABLE output
  )

macro(generate_test arg)
  set(name "${TEST_NAME}:${arg}")

  set(generated_line "add_test(\"${name}\" \"${TEST_EXECUTABLE}\" \"${arg}\")")
  string(APPEND script_output "${generated_line}\n")
endmacro()

set(script_output)
string(REPLACE "\n" ";" datatags "${output}")
foreach(line ${datatags})
  if(line MATCHES "^[^ ]* ([^ ]*) (.*)$")
    generate_test("${CMAKE_MATCH_1}:${CMAKE_MATCH_2}")
  elseif(line MATCHES "^[^ ]* ([^ ]*)$")
    generate_test("${CMAKE_MATCH_1}")
  endif()
endforeach()

file(WRITE "${TEST_OUTPUT_FILE}" "${script_output}")
