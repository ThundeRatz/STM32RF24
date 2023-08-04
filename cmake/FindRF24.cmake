set(RF24_PATH .lib/STM32RF24)

list(APPEND LIB_SOURCES
    ${RF24_PATH}/src/rf24_debug.c
    ${RF24_PATH}/src/rf24_plataform.c
    ${RF24_PATH}/src/rf24.c
)

list(APPEND LIB_SOURCES
    ${RF24_PATH}/inc
)
