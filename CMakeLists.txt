# Set up project
cmake_minimum_required(VERSION 3.2)

set(CMAKE_VERBOSE_MAKEFILE on)

cmake_policy(SET CMP0065 NEW)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
enable_testing()

set(CMAKE_CXX_STANDARD 14)



project(oxengstrings)
include(CMakeToolsHelpers OPTIONAL)
# Source files

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mx32"))
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
add_definitions(-pthread -D_FORTIFY_SOURCE=2 -g  -fstack-protector-all -Wl,-z,relro,-z,now)

add_definitions(-Wall -Wextra -Werror -ansi -pedantic -Wshadow -Weffc++ -Wformat -Wformat-security  -Wcast-align -Wcast-qual -Wvolatile-register-var  -Wwrite-strings -Wdisabled-optimization)
find_program( MEMORYCHECK_COMMAND valgrind )
set( MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full" )
set( MEMORYCHECK_SUPPRESSIONS_FILE "${PROJECT_SOURCE_DIR}/valgrind_suppress.txt" )

# find_package(BISON)
# if (NOT BISON_FOUND)
# 	message(SEND_ERROR "Could not find bison parser generator!")
# endif()



# FLEX_FOUND - true is flex executable is found
# FLEX_EXECUTABLE - the path to the flex executable
# FLEX_VERSION - the version of flex
# FLEX_LIBRARIES - The flex libraries
# FLEX_INCLUDE_DIRS - The path to the flex headers

# find_package(FLEX)
# if (NOT FLEX_FOUND)
# 	message(SEND_ERROR "Could not find flex scanner generator!")
# endif()
find_package(GOLD)
if (NOT GOLD_FOUND)
        message(SEND_ERROR "Could not find gold linker in binutils!")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold ")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--gdb-index -lpthread")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags -lpthread")
endif()

set(PROJECT_SRC
	src/main.cc
        src/generators.cc
	src/converters.cc
)
include_directories(include)
add_executable(${PROJECT_NAME} ${PROJECT_SRC})


add_test(
    NAME Testv${PROJECT_NAME}
    COMMAND  ${PROJECT_NAME}  -v
    )
  set_tests_properties (Testv${PROJECT_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "The current version is")

add_test(
    NAME Testh${PROJECT_NAME}
    COMMAND  ${PROJECT_NAME}  -h
    )
  set_tests_properties (Testh${PROJECT_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "Usage:")

add_test(
    NAME Testfile${PROJECT_NAME}
    COMMAND  ${PROJECT_NAME}  -i ../test/loc_str.doc
    )
  set_tests_properties (Testfile${PROJECT_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "Doc filename : ../test/loc_str.doc")
add_test(
    NAME Testmmaphash${PROJECT_NAME}
    COMMAND  ${PROJECT_NAME}  -i ../test/loc_str.doc
    )
  set_tests_properties (Testfile${PROJECT_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "Doc filename : ../test/loc_str.doc : 16471785882770998785")

add_test(
    NAME Testconfig${PROJECT_NAME}
    COMMAND  ${PROJECT_NAME}  -c ../test/config
    )
  set_tests_properties (Testconfig${PROJECT_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "Config directory : ../test/config")

  add_test(
    NAME Testdata${PROJECT_NAME}
    COMMAND  ${PROJECT_NAME}  -t ../test/data
    )
  set_tests_properties (Testdata${PROJECT_NAME} PROPERTIES PASS_REGULAR_EXPRESSION "Data directory : ../test/data")





set(CMAKE_INSTALL_PREFIX "/opt/oxeng/" CACHE PATH "..." FORCE)

install(TARGETS ${PROJECT_NAME} DESTINATION "bin")


include(InstallRequiredSystemLibraries)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Rémy Tomasetto") #required
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-dev")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Build_string is a tool used to generate legacy internationalized string for OXE.")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/opt/oxeng/")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "bin")
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
set(CPACK_SET_DESTDIR ON)

include(CPack)
