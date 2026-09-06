cmake_minimum_required(VERSION 3.16)
project(rhodes_daily_terminal VERSION 0.1.0 LANGUAGES C)

option(EPASS_NATIVE "Build with the electronic pass DRM/input backend" OFF)
set(EPASS_APPLICATIONS_ROOT "" CACHE PATH "Path to rhodesepass/epass-applications")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(DAILY_TERMINAL_PLATFORM src/platform.c)
if(EPASS_NATIVE)
  if(NOT EXISTS "${EPASS_APPLICATIONS_ROOT}/hal/CMakeLists.txt")
    message(FATAL_ERROR "EPASS_NATIVE requires -DEPASS_APPLICATIONS_ROOT=/path/to/epass-applications")
  endif()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBDRM REQUIRED libdrm)
  pkg_check_modules(FREETYPE REQUIRED freetype2)
  pkg_check_modules(EPASS_FONTS QUIET epass-fonts)
  add_subdirectory("${EPASS_APPLICATIONS_ROOT}/hal" "${CMAKE_BINARY_DIR}/epass-hal")
  add_subdirectory("${EPASS_APPLICATIONS_ROOT}/applications/common/epass_game"
                   "${CMAKE_BINARY_DIR}/epass-game")
  # Upstream uses clock_gettime/usleep; musl hides them in strict C11 mode.
  target_compile_definitions(hal_display PRIVATE _GNU_SOURCE)
  target_compile_definitions(epass_game PRIVATE _GNU_SOURCE)
  set(DAILY_TERMINAL_PLATFORM src/platform_epass.c)
endif()

add_executable(daily_terminal
  src/main.c
  src/app_ui.c
  src/storage.c
  ${DAILY_TERMINAL_PLATFORM}
)
target_include_directories(daily_terminal PRIVATE include)
if(EPASS_NATIVE)
  target_compile_definitions(daily_terminal PRIVATE EPASS_NATIVE=1)
  target_include_directories(daily_terminal PRIVATE ${FREETYPE_INCLUDE_DIRS})
  target_compile_options(daily_terminal PRIVATE ${FREETYPE_CFLAGS_OTHER})
  target_link_libraries(daily_terminal PRIVATE epass_game ${FREETYPE_LIBRARIES})
  if(EPASS_FONTS_FOUND)
    pkg_get_variable(EPASS_BODY_FONT epass-fonts font_body)
    if(EPASS_BODY_FONT)
      target_compile_definitions(daily_terminal PRIVATE DT_FONT_PATH="${EPASS_BODY_FONT}")
    endif()
  endif()
endif()

if(MSVC)
  target_compile_options(daily_terminal PRIVATE /W4)
else()
  target_compile_options(daily_terminal PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
  add_executable(daily_terminal_tests
    tests/test_app.c
    src/app_ui.c
    src/storage.c
  )
  target_include_directories(daily_terminal_tests PRIVATE include)
  add_test(NAME daily_terminal_tests COMMAND daily_terminal_tests)
endif()

install(TARGETS daily_terminal RUNTIME DESTINATION .)
install(FILES appconfig.json DESTINATION .)

find_package(Python3 COMPONENTS Interpreter QUIET)
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
  add_test(NAME manifest_validation
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_manifest.py
            ${CMAKE_CURRENT_SOURCE_DIR}/appconfig.json)
endif()
