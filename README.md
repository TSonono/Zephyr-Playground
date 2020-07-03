# Zephyr Playground

In this repo I will do some small projects to learn how to develop for the Zephyr RTOS.

## Background
I want to learn how to use the Zephyr RTOS for IoT applications. Let's start with some simple projects inspired from the samples available in the [Zephyr repo](https://github.com/zephyrproject-rtos/zephyr). All of the applications on this repo have been compiled with a Mac and are running on an __nRF52840 DK__.

## Get Things Running
- [Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html)

## Build

```bash
west build -p auto -b nrf52840dk_nrf52840 first-example-app
```

## Flash
```bash
west flash
```

## Debug with UART
```bash
picocom -e b -b [baudrate] [device]
```
Currently running with a baudrate of 115200. The device can be found using ls /dev/.
## Applications

Applications in this repo and their description.

### First Example App
Two threads, one spawned during compilation and one dynamically during runtime. Two semaphores used to control which of the threads will be running. When they run, an LED representing that thread will light up and it will print a debugging message.