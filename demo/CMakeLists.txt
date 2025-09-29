
cmake_minimum_required(VERSION 3.20)

project("RTMCClientLibraryDemo")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "Configuring RTMCClientLibraryDemo...")

include_directories(~/rtmc-client-library/include)

add_executable(epson_rtmc_demo epson_rtmc_demo.cpp create_reference.cpp)
target_link_libraries(epson_rtmc_demo ~/rtmc-client-library/build/librtmcclient.so)

add_executable(epson_rtmc_secure_demo epson_rtmc_secure_demo.cpp create_reference.cpp)
target_link_libraries(epson_rtmc_secure_demo ~/rtmc-client-library/build/librtmcclient.so)

add_executable(epson_rtmc_demo_io epson_rtmc_demo_io.cpp create_reference.cpp)
target_link_libraries(epson_rtmc_demo_io ~/rtmc-client-library/build/librtmcclient.so)


