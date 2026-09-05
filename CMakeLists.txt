cmake_minimum_required(VERSION 3.16)
project(rhodes_daily_terminal VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(daily_terminal
  src/main.c
  src/app.c
  src/storage.c
  src/platform.c
)
target_include_directories(daily_terminal PRIVATE include)

if(MSVC)
  target_compile_options(daily_terminal PRIVATE /W4)
else()
  target_compile_options(daily_terminal PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
  add_executable(daily_terminal_tests
    tests/test_app.c
    src/app.c
    src/storage.c
  )
  target_include_directories(daily_terminal_tests PRIVATE include)
  add_test(NAME daily_terminal_tests COMMAND daily_terminal_tests)
endif()

install(TARGETS daily_terminal RUNTIME DESTINATION .)
install(FILES appconfig.json DESTINATION .)

