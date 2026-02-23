cmake_minimum_required(VERSION 3.28 FATAL_ERROR)
project(wipeout-rewrite)

if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
	message(FATAL_ERROR "In-tree builds are not allowed.")
endif()

include(GNUInstallDirs)
include(CMakeDependentOption)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
	set(EMSCRIPTEN true)
endif()

set(platform_options "SDL2" "SOKOL")
set(PLATFORM "SDL2" CACHE STRING "Graphics platform to handle input/output")
set_property(CACHE PLATFORM PROPERTY STRINGS "${platform_options}")
if(NOT PLATFORM IN_LIST platform_options)
	message(FATAL_ERROR "PLATFORM must be one of ${platform_options}")
endif()

set(renderer_options "GL" "GLES2" "SOFTWARE")
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
option(USE_ASAN "Enable AddressSanitizer" OFF)

if (DEV_BUILD)
	set(PATH_ASSETS "${CMAKE_SOURCE_DIR}/")
	set(PATH_USERDATA "${CMAKE_SOURCE_DIR}/")
endif()

# 3RD PARTY LIBRARIES
find_package(PkgConfig REQUIRED)
find_package(OpenGL)
find_package(GLEW REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(cmocka REQUIRED)
pkg_check_modules(libprotobuf-c REQUIRED IMPORTED_TARGET libprotobuf-c)

include_directories(${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include)

# COMMON COMPLIER FLAGS, DEFINES
set(CMAKE_C_STANDARD 11)
add_compile_options(
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra>
)

if(DEV_BUILD AND NOT WIN32)
	add_compile_options(-g -Og)
	if(NOT WIN32)
		add_compile_options(--coverage)
		add_link_options(--coverage)
	endif()
endif()

if(USE_ASAN)
	add_compile_options(-fsanitize=address)
	add_link_options(-fsanitize=address)
endif()

# INTERNAL LIBRARIES
# PROTOBUF MSGS
add_subdirectory(gen)

# NETWORK LIBRARY TARGET
add_subdirectory(network)

# SERVER TARGET
add_subdirectory(server)

# MAIN GAME TARGET
set(common_src
	src/wipeout/camera.c
	src/wipeout/server_com.c
	src/wipeout/droid.c
	src/wipeout/game.c
	src/wipeout/hud.c
	src/wipeout/image.c
	src/wipeout/ingame_menus.c
	src/wipeout/intro.c
	src/wipeout/main_menu.c
	src/wipeout/menu.c
	src/wipeout/object.c
	src/wipeout/particle.c
	src/wipeout/race.c
	src/wipeout/scene.c
	src/wipeout/sfx.c
	src/wipeout/ship.c
	src/wipeout/ship_ai.c
	src/wipeout/ship_player.c
	src/wipeout/title.c
	src/wipeout/track.c
	src/wipeout/ui.c
	src/wipeout/weapon.c
	src/input.c
	src/mem.c
	src/system.c
	src/types.c
	src/utils.c

	packaging/windows/wipeout.exe.manifest
	packaging/windows/wipeout.rc
)

add_executable(wipeout WIN32 ${common_src})
target_include_directories(wipeout PRIVATE src)
target_include_directories(wipeout SYSTEM PRIVATE src/libs)

target_compile_definitions(wipeout PRIVATE
	$<$<BOOL:${PATH_ASSETS}>:-DPATH_ASSETS=${PATH_ASSETS}>
	$<$<BOOL:${PATH_USERDATA}>:-DPATH_USERDATA=${PATH_USERDATA}>
)

# COMMON LIBRARIES
target_link_libraries(wipeout PRIVATE network)

if(WIN32)
	target_compile_definitions(wipeout PRIVATE
		"NOMINMAX"
		"_USE_MATH_DEFINES"
		"_CRT_SECURE_NO_WARNINGS"
	)
elseif(APPLE)
	target_compile_definitions(wipeout PRIVATE
		"_THREAD_SAFE"
		"GL_SILENCE_DEPRECATION"
	)
	target_link_libraries(wipeout PUBLIC "-framework Foundation")
	set_source_files_properties(src/platform_sokol.c PROPERTIES COMPILE_FLAGS "-x objective-c")
	if("${PLATFORM}" STREQUAL SOKOL)
		target_link_libraries(wipeout PUBLIC
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
	if (NOT TARGET SDL2::Main)
		add_library(SDL2::Main INTERFACE IMPORTED)
	endif()
	set_target_properties(OpenGL::GL PROPERTIES
		IMPORTED_LIBNAME "GL"
	)
	set_target_properties(GLEW::GLEW PROPERTIES
		IMPORTED_LIBNAME "GLEW"
	)
	set_target_properties(SDL2::Main PROPERTIES
		IMPORTED_LIBNAME "SDL2"
		INTERFACE_COMPILE_OPTIONS "SHELL:-s USE_SDL=2"
		INTERFACE_LINK_LIBRARIES  "SHELL:-s USE_SDL=2"
	)

	target_link_options(wipeout PRIVATE
		"SHELL:-s ALLOW_MEMORY_GROWTH=1"
		"SHELL:-s ENVIRONMENT=web"
		"SHELL:-s FORCE_FILESYSTEM"
		"SHELL:--preload-file ${CMAKE_SOURCE_DIR}/wipeout/@/wipeout"
	)
	if(MINIMAL_BUNDLE)
		target_link_options(wipeout PRIVATE
			"SHELL:--exclude-file ${CMAKE_SOURCE_DIR}/wipeout/music"
			"SHELL:--exclude-file ${CMAKE_SOURCE_DIR}/intro.mpeg"
		)
	endif()

	configure_file("${CMAKE_SOURCE_DIR}/src/wasm-index.html" "game.html" COPYONLY)
elseif(UNIX)
	target_link_libraries(wipeout PUBLIC m)
	if ("${PLATFORM}" STREQUAL "SOKOL" AND "${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
		find_package(Threads REQUIRED)
		find_package(X11 REQUIRED)
		find_package(ALSA REQUIRED)
		target_link_libraries(wipeout PUBLIC
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
	target_compile_definitions(wipeout PRIVATE "RENDERER_GL")
	target_sources(wipeout PRIVATE src/render_gl.c)

	target_include_directories(wipeout PUBLIC ${OPENGL_INCLUDE_DIR})
	if ("${RENDERER}" STREQUAL "GLES2")
		target_compile_definitions(wipeout PRIVATE "USE_GLES2")
		if (TARGET OpenGL::GLES2)
			target_link_libraries(wipeout PUBLIC OpenGL::GLES2)
		endif()
	endif()

	if(USE_GLVND AND TARGET OpenGL::OpenGL)
		target_link_libraries(wipeout PUBLIC OpenGL::OpenGL)
	else()
		target_link_libraries(wipeout PUBLIC OpenGL::GL)
	endif()

	if(NOT APPLE)
		target_include_directories(wipeout PRIVATE ${GLEW_INCLUDE_DIRS})
		target_link_libraries(wipeout PRIVATE GLEW::GLEW)
	endif()
elseif("${RENDERER}" STREQUAL "SOFTWARE")
	target_compile_definitions(wipeout PRIVATE "RENDERER_SOFTWARE")
	target_sources(wipeout PRIVATE src/render_software.c)
endif()

if("${PLATFORM}" STREQUAL SDL2)
	target_sources(wipeout PRIVATE src/platform_sdl.c)
	target_link_libraries(wipeout 
		PRIVATE 
		$<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
		$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
	)
elseif("${PLATFORM}" STREQUAL SOKOL)
	target_sources(wipeout PRIVATE src/platform_sokol.c)
endif()

# UNIT TESTS TARGET
add_subdirectory(tests)

install(TARGETS wipeout)
