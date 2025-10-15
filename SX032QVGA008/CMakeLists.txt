add_library(SX032QVGA008 SX032QVGA008.cpp GraphicsDisplay.cpp SPI_TFT_ILI9341ext.cpp TextDisplay.cpp)
target_include_directories(SX032QVGA008 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(SX032QVGA008 PUBLIC mbed-os SPI_TFT_ILI9341ext)
target_link_libraries(SX032QVGA008 PUBLIC mbed-baremetal)
