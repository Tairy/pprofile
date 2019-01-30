cmake_minimum_required(VERSION 3.3)
project(hank)

add_custom_target(makefile COMMAND make && make install
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})