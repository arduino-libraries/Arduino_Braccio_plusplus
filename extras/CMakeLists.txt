##########################################################################
cmake_minimum_required(VERSION 3.16)
##########################################################################
project(lvgl)
##########################################################################
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
##########################################################################
add_library(${PROJECT_NAME} STATIC "")
##########################################################################
set(LVGL_ROOT_DIR ${CMAKE_SOURCE_DIR}/../../lvgl)
set(LV_CONF_DIR ${CMAKE_SOURCE_DIR}/../src)
##########################################################################
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${LVGL_ROOT_DIR} ${LV_CONF_DIR})
##########################################################################
file(GLOB_RECURSE LVGL_SOURCES ${LVGL_ROOT_DIR}/src/*.c)
target_sources(${PROJECT_NAME} PUBLIC ${LVGL_SOURCES})
##########################################################################
target_compile_options(${PROJECT_NAME} PRIVATE -mcpu=cortex-m0plus -fPIC)
##########################################################################
file(WRITE ${CMAKE_BINARY_DIR}/cp-lvgl-hdr.cmake
  "file(COPY ${LVGL_ROOT_DIR}/src DESTINATION ../../src/lib/lvgl FILES_MATCHING PATTERN *.h)\n"
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${CMAKE_BINARY_DIR}/liblvgl.a ../../src/cortex-m0plus/liblvgl.a COMMENT "Copying libvgl.a")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${LVGL_ROOT_DIR}/lvgl.h ../../src/lib/lvgl  COMMENT "Copying lvgl.h")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cp-lvgl-hdr.cmake COMMENT "Copying lvgl/src/*.h")
##########################################################################
