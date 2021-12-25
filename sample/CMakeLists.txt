cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif()
endif()

project(libAppSettings_sample)

include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "libAppSettings_sample")
set(VITA_TITLEID  "GRVA00099")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -fno-builtin")

link_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
	)

add_executable(${PROJECT_NAME}
	src/main.c
	)

target_link_libraries(${PROJECT_NAME}
	SceSysmodule_stub
	AppSettings_stub
    )

vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)
