cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})
project(non_generic_example_app)

target_sources(app
               PRIVATE src/main.c)
