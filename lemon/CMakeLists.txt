#
# Lemon parser generator build specification.
#

cmake_minimum_required(VERSION 3.5)

project(lemon)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

add_executable(lemon lemon.c)

# Remove top-level compile options since they are too strict for lemon.
set_property(TARGET lemon
    PROPERTY COMPILE_OPTIONS "")
