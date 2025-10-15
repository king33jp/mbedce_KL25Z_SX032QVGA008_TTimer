add_library(MMA8451Q MMA8451Q.cpp)
target_include_directories(MMA8451Q PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(MMA8451Q PUBLIC mbed-baremetal)
