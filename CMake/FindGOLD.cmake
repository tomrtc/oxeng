set(GOLD_FOUND FALSE)

  find_program(GOLD_EXECUTABLE NAMES gold ld.gold DOC "path to gold")

  if(GOLD_EXECUTABLE)
    set(GOLD_FOUND TRUE)
    execute_process(COMMAND ${GOLD_EXECUTABLE} --version
      OUTPUT_VARIABLE GOLD_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    message(STATUS "Found gold: ${GOLD_EXECUTABLE}") 
  else()
    message(STATUS "Could not find gold linker. Using the default")
  endif()
