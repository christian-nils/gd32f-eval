set(SOURCES
    src/main.c
)

add_executable(gd32f3-eval
    ../../startup/startup_gd32f30x_hd.S
    ${SOURCES}
)

target_link_libraries(gd32f3-eval
    gd32f30x
)

target_include_directories(gd32f3-eval
    PUBLIC
)

set_target_linker(gd32f3-eval  ${CMAKE_CURRENT_SOURCE_DIR}/../../startup/GD32F303RC.ld)

post_build(gd32f3-eval)