cmake_minimum_required(VERSION 3.22)
project(wipeout_android LANGUAGES C CXX)

set(WIPEOUT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../..")
get_filename_component(WIPEOUT_ROOT "${WIPEOUT_ROOT}" ABSOLUTE)

set(SDL2_SOURCE_DIR "${WIPEOUT_ROOT}/third_party/SDL" CACHE PATH "Path to SDL2 source")
set(SDL2_GIT_TAG "release-2.32.10" CACHE STRING "SDL2 git tag")

set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)

if(EXISTS "${SDL2_SOURCE_DIR}/CMakeLists.txt")
	add_subdirectory("${SDL2_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/sdl2")
else()
	include(FetchContent)
	FetchContent_Declare(SDL2
		GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
		GIT_TAG ${SDL2_GIT_TAG}
		GIT_SHALLOW TRUE
		SOURCE_DIR "${SDL2_SOURCE_DIR}"
		BINARY_DIR "${CMAKE_BINARY_DIR}/sdl2"
	)
	FetchContent_MakeAvailable(SDL2)
endif()

include("${WIPEOUT_ROOT}/cmake/wipeout_sources.cmake")

add_library(main SHARED ${WIPEOUT_COMMON_SRC})
set_property(TARGET main PROPERTY C_STANDARD 11)

target_sources(main PRIVATE
	"${WIPEOUT_ROOT}/src/platform_sdl.c"
	"${WIPEOUT_ROOT}/src/render_gl.c"
)

target_include_directories(main PRIVATE "${WIPEOUT_ROOT}/src")
target_include_directories(main SYSTEM PRIVATE "${WIPEOUT_ROOT}/src/libs")

target_compile_definitions(main PRIVATE RENDERER_GL USE_GLES2 PLM_USE_SDL_RWOPS)

find_library(log-lib log)
find_library(android-lib android)
find_library(egl-lib EGL)
find_library(gles-lib GLESv2)

if(TARGET SDL2::SDL2)
	target_link_libraries(main SDL2::SDL2)
elseif(TARGET SDL2)
	target_link_libraries(main SDL2)
endif()

target_link_libraries(main
	${log-lib}
	${android-lib}
	${egl-lib}
	${gles-lib}
	m
)
