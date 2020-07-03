cmake_minimum_required(VERSION 3.13.1)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(second_example_app)

target_sources(app
               PRIVATE src/main.c
               PRIVATE src/app_gpio.c)

target_include_directories(app
                           PRIVATE inc)
