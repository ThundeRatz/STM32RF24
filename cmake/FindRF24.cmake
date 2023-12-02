set(RF24_PATH ./lib/STM32RF24)

list(APPEND LIB_SOURCES
    ${RF24_PATH}/src/rf24_debug.c
    ${RF24_PATH}/src/rf24_platform.c
    ${RF24_PATH}/src/rf24.c
)

list(APPEND LIB_INCLUDE_DIRECTORIES
    ${RF24_PATH}/inc
)
