# set(SOURCES
#     USART/printf/src/main.c
#     gd32f3_eval.c

# )

set(SOURCES
    USART/DMA_transmitter&receiver/src/main.c
    gd32f3_eval.c
)

# set(SOURCES
#     gd32f3_eval.c
#     USART/Transmitter&receiver_interrupt/src/main.c
#     USART/Transmitter&receiver_interrupt/src/gd32f30x_it.c
# )

# set(SOURCES
#     gd32f3_eval.c
#     USART/DMA_transmitter&receiver_interrupt/src/main.c
#     USART/DMA_transmitter&receiver_interrupt/src/gd32f30x_it.c
# )

add_executable(gd32f3-eval
    startup/startup_gd32f30x_hd.S
    ${SOURCES}
)

target_link_libraries(gd32f3-eval
    gd32f30x
)

target_include_directories(gd32f3-eval
    PUBLIC
    gd32f3_eval.h
    USART/DMA_transmitter&receiver/include
)

set_target_linker(gd32f3-eval  ${CMAKE_CURRENT_SOURCE_DIR}/startup/GD32F303RC.ld)

post_build(gd32f3-eval)