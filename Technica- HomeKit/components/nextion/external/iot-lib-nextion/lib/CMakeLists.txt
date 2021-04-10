file(GLOB srcsNEXTION "src/nextion/*.c")
file(GLOB srcsRGB565 "src/rgb565/*.c")

list(APPEND srcs ${srcsNEXTION})
list(APPEND srcs ${srcsRGB565})

add_library(iot-lib-nextion STATIC ${srcs})

target_include_directories(iot-lib-nextion PUBLIC "include")
target_include_directories(iot-lib-nextion PRIVATE "private_include")