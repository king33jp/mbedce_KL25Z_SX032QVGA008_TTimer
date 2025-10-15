add_library(RTC_S35190 RTC_S35190.cpp)
target_include_directories(RTC_S35190 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(RTC_S35190 PUBLIC mbed-baremetal)