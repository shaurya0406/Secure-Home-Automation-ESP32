file(GLOB srcsTstNEXTION "nextion/*.cpp")
file(GLOB srcsTstRGB565 "rgb565/*.cpp")

list(APPEND srcsTst "test_runner.cpp")
list(APPEND srcsTst ${srcsTstNEXTION})
list(APPEND srcsTst ${srcsTstRGB565})

add_executable(test_app ${srcsTst})

include(FetchContent)

FetchContent_Declare(
    CppUTest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG        latest-passing-build
)

set(TESTS OFF CACHE BOOL "Switch off CppUTest Test build")

FetchContent_MakeAvailable(CppUTest)

target_include_directories(test_app PUBLIC ".")
target_include_directories(test_app PUBLIC "../lib/include")
target_include_directories(test_app PRIVATE "../lib/private_include")

target_link_libraries(test_app PRIVATE iot-lib-nextion)
target_link_libraries(test_app PRIVATE CppUTest CppUTestExt)
