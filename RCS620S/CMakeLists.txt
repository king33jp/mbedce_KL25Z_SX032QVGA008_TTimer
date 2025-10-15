add_library(RCS620S RCS620S.cpp)
target_include_directories(RCS620S PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(RCS620S PUBLIC mbed-baremetal)