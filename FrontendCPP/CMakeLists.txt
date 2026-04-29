cmake_minimum_required(VERSION 3.28.3)
project(WateRMQCPP)

# Find required packages
find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SRCS 
    WateRMQCPP.cpp 
    SemaphoreCPP.cpp 
    WateRMQ.cpp
)

# Create executable
add_executable(main ${SRCS})

# Include directories
target_include_directories(main PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${nlohmann_json_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(main PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    nlohmann_json::nlohmann_json
    rt  # for shm_open
    pthread
)

# Compiler options
target_compile_options(main PRIVATE -Wall -Wextra -g)

# Install
install(TARGETS main DESTINATION bin)
