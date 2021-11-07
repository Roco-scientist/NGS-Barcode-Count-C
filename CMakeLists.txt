 cmake_minimum_required(VERSION 3.10)
 project(barcode-count VERSION "0.2.1")
 # add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/src/gzstream/gzstream.o
 #                   COMMAND make run
 #                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/gzstream
 # )
 set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
 add_subdirectory(src)
