set(SOURCES
    ../../gd32f3_eval.c
    ../../startup/tx_initialize_low_level.S
    src/main.c
    src/microros_time.c
    src/microros_transports.c
    src/newlib_nano.c
    src/gd32f30x_it.c
    ../../../../lib/microros/micro_ros_src/src/rclc/rclc/src/rclc/publisher.c
    ../../../../lib/microros/micro_ros_src/src/rcl/rcl/src/rcl/publisher.c
)

add_executable(gd32f3-eval
    ../../startup/startup_gd32f30x_hd.S
    ${SOURCES}
)

target_link_libraries(gd32f3-eval
    gd32f30x
    azrtos::threadx
    libmicroros-prebuilt
)

target_include_directories(gd32f3-eval
    PUBLIC
    ../..
    ./include
)

set_target_linker(gd32f3-eval  ${CMAKE_CURRENT_SOURCE_DIR}/../../startup/GD32F303RC.ld)

post_build(gd32f3-eval)