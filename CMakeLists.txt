cmake_minimum_required(VERSION 3.15)

project(edlaaf CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS program_options ) 

include(CMakePrintHelpers)
cmake_print_variables(CMAKE_TOOLCHAIN_FILE)
cmake_print_variables(Boost_INCLUDE_DIR Boost_LIBRARIES )

set(AAF_INCLUDE_DIR "C:/views/AAF-src-1.2.0/AAF/out/shared/include")
set(AAF_LIB_DIR "C:/views/AAF-src-1.2.0/AAF/out/target/Win64-x64/RelWithDebInfo/RefImpl/RelWithDebInfo")

add_subdirectory ("edlaaf")
