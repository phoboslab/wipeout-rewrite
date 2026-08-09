cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(wipeout-rewrite LANGUAGES C)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(FATAL_ERROR "In-tree builds are not allowed.")
endif()

if(NOT CMAKE_GENERATOR MATCHES "^Visual Studio")
include(GNUInstallDirs)
endif()
include(CMakeDependentOption)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
	set(EMSCRIPTEN true)
endif()

set(platform_options "SDL2" "SOKOL" "NULL")
set(PLATFORM "SDL2" CACHE STRING "Graphics platform to handle input/output")
set_property(CACHE PLATFORM PROPERTY STRINGS "${platform_options}")
if(NOT PLATFORM IN_LIST platform_options)
	message(FATAL_ERROR "PLATFORM must be one of ${platform_options}")
endif()

set(renderer_options "GL" "GLES2" "SOFTWARE" "NULL")
set(RENDERER "GL" CACHE STRING "Graphics rendering backend")
set_property(CACHE RENDERER PROPERTY STRINGS "${renderer_options}")
if(NOT RENDERER IN_LIST renderer_options)
	message(FATAL_ERROR "RENDERER must be one of ${renderer_options}")
endif()
if("${RENDERER}" MATCHES "GL(ES2)?")
	set(using_gl true)
endif()
if("${CMAKE_SYSTEM_NAME}" MATCHES "(Linux|FreeBSD)")
	set(has_glvnd true)
endif()

cmake_dependent_option(USE_GLVND "Link against modern GLVND ABIs" ON "using_gl;has_glvnd" OFF)
cmake_dependent_option(MINIMAL_BUNDLE "Do not include music/movies for web builds" OFF "EMSCRIPTEN" OFF)
option(PATH_ASSETS "Path to where the game assets should be located.")
option(PATH_USERDATA "Path to where user data (e.g. game saves) should be located.")
option(DEV_BUILD "Set asset/userdata paths to the source directory for testing" OFF)
if (DEV_BUILD)
	set(PATH_ASSETS "${CMAKE_SOURCE_DIR}/")
	set(PATH_USERDATA "${CMAKE_SOURCE_DIR}/")
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED OFF)
set(CMAKE_C_EXTENSIONS ON)

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE )
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "${CMAKE_HOST_SYSTEM_NAME} ${CMAKE_HOST_SYSTEM_PROCESSOR} ${CMAKE_C_COMPILER_ID}/${CMAKE_C_COMPILER_FRONTEND_VARIANT} ${PLATFORM} ${RENDERER} => ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR} ${CMAKE_C_COMPILER_ID}/${CMAKE_C_COMPILER_FRONTEND_VARIANT} ${PLATFORM} ${RENDERER}")

add_compile_definitions(_FILE_OFFSET_BITS=64)
add_compile_definitions($<$<CONFIG:Debug>:DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:NDEBUG=1>)
add_compile_definitions($<$<CONFIG:Debug>:_DEBUG=1> $<$<NOT:$<CONFIG:Debug>>:_NDEBUG=1>)

if (CMAKE_C_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wno-gnu-alignof-expression)
endif()

if (CMAKE_C_COMPILER_FRONTEND_VARIANT MATCHES "GNU|AppleClang")
	if( NOT CMAKE_BUILD_OPTLEVEL )
		set( CMAKE_BUILD_OPTLEVEL 3 CACHE STRING "Choose the optimization level." FORCE )
	endif()
	add_compile_options(-O${CMAKE_BUILD_OPTLEVEL})
	if (NOT CMAKE_CROSSCOMPILING)
		add_compile_options(-march=native)

		if (${SYSTEM_PROCESSOR_UPPER} MATCHES "X64|X86_64|AMD64")
			add_compile_options(-mfpmath=sse)
		endif()
		if (${SYSTEM_PROCESSOR_UPPER} MATCHES "PPC|POWERPC|PPC64|PPC64LE")
			include(CheckCSourceCompiles)

			set(CMAKE_REQUIRED_FLAGS "-maltivec")
			check_c_source_compiles("#include <altivec.h> int main() { vector int v = (vector int){0}; return 0; }" HAVE_ALTIVEC)

			if(HAVE_ALTIVEC)
				add_compile_options(-maltivec)
			endif()
		endif()
	endif()
	add_compile_options(-ftree-vectorize)
	add_compile_options(-fopenmp)
	if(NOT CMAKE_C_COMPILER_FRONTEND_VARIANT MATCHES "AppleClang")
		add_compile_options(-fopenmp-simd)
	endif()
	add_compile_options(-fno-asynchronous-unwind-tables)
	if (CMAKE_BUILD_TYPE MATCHES Debug)
		add_compile_options(-g)
	else()
		# Use IPO/LTO if available
		if(NOT ${CMAKE_VERSION} VERSION_LESS "3.9.0")
			cmake_policy(SET CMP0069 NEW)
			include(CheckIPOSupported)
			check_ipo_supported(RESULT result)
			if(result)
				set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
				message(STATUS "Using IPO/LTO.")
			endif()
		endif()
	endif()
	add_compile_options(-Wall)
	add_compile_options(-Wextra)
	add_compile_options(-Wno-write-strings)
	add_compile_options(-Wno-unused-result)
	add_compile_options(-Wno-unused-variable)
	add_compile_options(-Wno-unused-but-set-variable)
	add_compile_options(-Wno-unused-parameter)
	add_compile_options(-Wno-missing-field-initializers)
	add_compile_options(-Wunreachable-code)		# Warn about unreachable code paths
	add_compile_options(-fpermissive)
	add_compile_options(-ffunction-sections)	# Put each function in its own section
	add_compile_options(-fdata-sections)		# Put each variable in its own section
	if (CMAKE_BUILD_TYPE MATCHES ReleaseStrip)
		add_link_options(-Wl,--gc-sections)
		add_link_options(-Wl,-s)
	endif()
else (CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
	string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
	string(REPLACE "/RTC1" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	add_compile_options(/openmp:experimental)
	add_compile_options(/Zc:__STDC__)
	add_compile_options(/std:clatest)
	add_compile_options(/Zc:preprocessor)
	add_compile_options(/Z7)
	add_compile_options(/Gy)					# Function-level linking (puts each function in its own section)
	add_compile_options(/Gw)					# Optimize global data (puts each variable in its own section)
	add_compile_options(/W4>
)
	if (NOT CMAKE_CROSSCOMPILING)
		add_compile_definitions(_CRT_SECURE_NO_WARNINGS _WIN32)
		if (CMAKE_VS_PLATFORM_NAME MATCHES "x64|Win32")
			#add_compile_options(/Gv)
		endif()
	endif()
	if (CMAKE_BUILD_TYPE MATCHES ReleaseStrip)
		add_link_options(/DEBUG:NONE)
		add_link_options(/OPT:REF)				# Eliminate unreferenced functions and data
		add_link_options(/VERBOSE:REF)			# Print out everything the linker throws away
	endif()
	if (CMAKE_BUILD_TYPE MATCHES Debug)
		add_compile_options(/Zi)
		add_link_options(/DEBUG:FULL)
	else()
		if( NOT CMAKE_BUILD_OPTLEVEL )
			set( CMAKE_BUILD_OPTLEVEL 2 CACHE STRING "Choose the optimization level." FORCE )
		endif()
		add_compile_options(/O${CMAKE_BUILD_OPTLEVEL})
	endif()
	add_link_options(/INCREMENTAL:NO)
endif()

find_package(OpenGL)
find_package(GLEW)
find_package(SDL2)

set(common_src
	src/wipeout/camera.c
	src/wipeout/camera.h
	src/wipeout/droid.c
	src/wipeout/droid.h
	src/wipeout/game.c
	src/wipeout/game.h
	src/wipeout/hud.c
	src/wipeout/hud.h
	src/wipeout/image.c
	src/wipeout/image.h
	src/wipeout/ingame_menus.c
	src/wipeout/ingame_menus.h
	src/wipeout/intro.c
	src/wipeout/intro.h
	src/wipeout/main_menu.c
	src/wipeout/main_menu.h
	src/wipeout/menu.c
	src/wipeout/menu.h
	src/wipeout/object.c
	src/wipeout/object.h
	src/wipeout/particle.c
	src/wipeout/particle.h
	src/wipeout/race.c
	src/wipeout/race.h
	src/wipeout/scene.c
	src/wipeout/scene.h
	src/wipeout/sfx.c
	src/wipeout/sfx.h
	src/wipeout/ship.c
	src/wipeout/ship.h
	src/wipeout/ship_ai.c
	src/wipeout/ship_ai.h
	src/wipeout/ship_player.c
	src/wipeout/ship_player.h
	src/wipeout/title.c
	src/wipeout/title.h
	src/wipeout/track.c
	src/wipeout/track.h
	src/wipeout/ui.c
	src/wipeout/ui.h
	src/wipeout/weapon.c
	src/wipeout/weapon.h
	src/input.c
	src/input.h
	src/mem.c
	src/mem.h
	src/platform.h
	src/render.h
	src/system.c
	src/system.h
	src/types.c
	src/types.h
	src/utils.c
	src/utils.h

	packaging/windows/wipeout.exe.manifest
	packaging/windows/wipeout.rc
)

add_executable(wipegame WIN32 ${common_src})
target_include_directories(wipegame PRIVATE src)
target_include_directories(wipegame SYSTEM PRIVATE src/libs)
target_compile_definitions(wipegame PRIVATE
	$<$<BOOL:${PATH_ASSETS}>:-DPATH_ASSETS=${PATH_ASSETS}>
	$<$<BOOL:${PATH_USERDATA}>:-DPATH_USERDATA=${PATH_USERDATA}>
)

if(WIN32)
	target_compile_definitions(wipegame PRIVATE
		"NOMINMAX"
		"_USE_MATH_DEFINES"
		"_CRT_SECURE_NO_WARNINGS"
	)
elseif(APPLE)
	target_compile_definitions(wipegame PRIVATE
		"_THREAD_SAFE"
		"GL_SILENCE_DEPRECATION"
	)
	target_link_libraries(wipegame PUBLIC "-framework Foundation")
	set_source_files_properties(src/platform_sokol.c PROPERTIES COMPILE_FLAGS "-x objective-c")
	if("${PLATFORM}" STREQUAL SOKOL)
		target_link_libraries(wipegame PUBLIC
			"-framework Cocoa"
			"-framework QuartzCore"
			"-framework AudioToolbox"
		)
	endif()
elseif(EMSCRIPTEN)
	# Emscripten's CMake modules don't define targets like the standard
	# ones do, so we define them ourselves here.
	add_library(GLEW::GLEW INTERFACE IMPORTED)
	add_library(OpenGL::GL INTERFACE IMPORTED)
	if (NOT TARGET SDL2::SDL2main)
		add_library(SDL2::SDL2main INTERFACE IMPORTED)
	endif()
	set_target_properties(OpenGL::GL PROPERTIES
		IMPORTED_LIBNAME "GL"
	)
	set_target_properties(GLEW::GLEW PROPERTIES
		IMPORTED_LIBNAME "GLEW"
	)
	set_target_properties(SDL2::SDL2main PROPERTIES
		IMPORTED_LIBNAME "SDL2"
		INTERFACE_COMPILE_OPTIONS "SHELL:-s USE_SDL=2"
		INTERFACE_LINK_LIBRARIES  "SHELL:-s USE_SDL=2"
	)

	target_link_options(wipegame PRIVATE
		"SHELL:-s ALLOW_MEMORY_GROWTH=1"
		"SHELL:-s ENVIRONMENT=web"
		"SHELL:-s FORCE_FILESYSTEM"
		"SHELL:--preload-file ${CMAKE_SOURCE_DIR}/wipeout/@/wipeout"
	)
	if(MINIMAL_BUNDLE)
		target_link_options(wipegame PRIVATE
			"SHELL:--exclude-file ${CMAKE_SOURCE_DIR}/wipeout/music"
			"SHELL:--exclude-file ${CMAKE_SOURCE_DIR}/intro.mpeg"
		)
	endif()

	configure_file("${CMAKE_SOURCE_DIR}/src/wasm-index.html" "game.html" COPYONLY)
elseif(UNIX)
	target_link_libraries(wipegame PUBLIC m)
	if ("${PLATFORM}" STREQUAL "SOKOL" AND "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
		find_package(Threads REQUIRED)
		find_package(X11 REQUIRED)
		find_package(ALSA REQUIRED)
		target_link_libraries(wipegame PUBLIC
			X11::X11
			X11::Xcursor
			Threads::Threads
			X11::Xi
			dl
			ALSA::ALSA
		)
	endif()
endif()

if(using_gl)
	target_compile_definitions(wipegame PRIVATE "RENDERER_GL")
	target_sources(wipegame PRIVATE src/render_gl.c)

	target_include_directories(wipegame PUBLIC ${OPENGL_INCLUDE_DIR})
	if ("${RENDERER}" STREQUAL "GLES2")
		target_compile_definitions(wipegame PRIVATE "USE_GLES2")
		if (TARGET OpenGL::GLES2)
			target_link_libraries(wipegame PUBLIC OpenGL::GLES2)
		endif()
	endif()

	if(USE_GLVND AND TARGET OpenGL::OpenGL)
		target_link_libraries(wipegame PUBLIC OpenGL::OpenGL)
	else()
		target_link_libraries(wipegame PUBLIC OpenGL::GL)
	endif()

	if(NOT APPLE)
		target_include_directories(wipegame PRIVATE ${GLEW_INCLUDE_DIRS})
		target_link_libraries(wipegame PRIVATE GLEW::GLEW)
	endif()
elseif("${RENDERER}" STREQUAL "SOFTWARE")
	target_compile_definitions(wipegame PRIVATE "RENDERER_SOFTWARE")
	target_sources(wipegame PRIVATE src/render_software.c)
elseif("${RENDERER}" STREQUAL "NULL")
	target_compile_definitions(wipegame PRIVATE "RENDERER_NULL")
	target_sources(wipegame PRIVATE src/render_null.c)
endif()

if("${PLATFORM}" STREQUAL SDL2)
	target_sources(wipegame PRIVATE src/platform_sdl.c)
	target_link_libraries(wipegame PUBLIC SDL2::SDL2 SDL2::SDL2main)
elseif("${PLATFORM}" STREQUAL SOKOL)
	target_sources(wipegame PRIVATE src/platform_sokol.c)
elseif("${PLATFORM}" STREQUAL NULL)
	target_sources(wipegame PRIVATE src/platform_null.c)
endif()

install(TARGETS wipegame)
