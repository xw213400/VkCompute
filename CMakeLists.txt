cmake_minimum_required (VERSION 3.3)
project(ggml VERSION 0.1.0)

set(CMAKE_C_STANDARD   17)
set(CMAKE_CXX_STANDARD 17)

file(GLOB VK_COMPUTE_SRC VkCompute/*.cpp)
file(GLOB VK_COMPUTE_INC VkCompute/*.h)

message(STATUS "VK_COMPUTE_SRC ---> " ${VK_COMPUTE_SRC})
message(STATUS "VK_COMPUTE_INC ---> " ${VK_COMPUTE_INC})

find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    set(TEST_TARGET test-vulkan)
    add_executable(${TEST_TARGET} tests/${TEST_TARGET}.cpp ${VK_COMPUTE_SRC})
    target_include_directories(${TEST_TARGET} PUBLIC ${Vulkan_INCLUDE_DIR} ${VK_COMPUTE_INC})
    target_link_libraries(${TEST_TARGET} PRIVATE ${Vulkan_LIBRARIES})
endif()