cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nrf-ds4)

FILE(GLOB SOURCES src/*.c)
target_sources(app PRIVATE ${SOURCES})
