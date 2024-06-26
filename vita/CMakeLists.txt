cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(Allefresher_vita)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -fno-inline")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(Allefresher_vita
  main.c
  reader.c
  clib.c
)

target_link_libraries(Allefresher_vita
  taihen_stub
  SceLibKernel_stub
  SceDebugForDriver_stub
  SceIofilemgr_stub
)

set_target_properties(Allefresher_vita
  PROPERTIES LINK_FLAGS "-nostdlib"
)

vita_create_self(Allefresher_vita.suprx Allefresher_vita CONFIG exports.yml UNSAFE)