
cmake_minimum_required(VERSION 3.20)

project("RTMCClientLibrary" VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)
find_library(TINYXML2_LIBRARY tinyxml2)

link_directories(~/FastDDS/install/fastcdr/lib)
link_directories(~/FastDDS/install/fastrtps/lib)
link_directories(~/FastDDS/install/foonathan_memory_vendor/lib)

message(STATUS "Configuring RTMCClientLibrary...")

add_library(rtmcclient SHARED src/DDSCommunicationPubSubTypes.cxx src/DDSCommunication.cxx src/rtmc_client.cpp src/DDSCommunicationCore.cxx src/rb_weight_inertia_param.cpp src/dataformat_converter.cpp)
target_include_directories(rtmcclient PRIVATE ~/FastDDS/install/fastcdr/include ~/FastDDS/install/fastrtps/include ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(rtmcclient PRIVATE libfastcdr.a libfastrtps.a libfoonathan_memory-0.7.3.a OpenSSL::SSL OpenSSL::Crypto ${TINYXML2_LIBRARY})
