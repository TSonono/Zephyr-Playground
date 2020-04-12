# Zephyr Playground

In this repo I will do some small projects to learn how to develop for the Zephyr RTOS.

## Background
I want to learn how to use the Zephyr RTOS for IoT applications. Let's start with some simple projects inspired from the samples available in the [Zephyr repo](https://github.com/zephyrproject-rtos/zephyr). All of the applications on this repo have been compiled with a Mac and are running on an __nRF52840 DK__.

## Get Things Running
- [Zephyr Getting Started Guide](https://docs.zephyrproject.org/latest/getting_started/index.html)
- Clone this repo in the zephyrproject directory created if you ran `west init zephyrproject`
- The requirements.txt file in this repo is not necessarily aligned with the on the zephyr repo. You can use the one included in the zephyr repo and then do `pip install pyrtt-viewer` (I do this in an virtual environment). The requirements.txt file in this repo includes pyrtt-viewer

## Build

```bash
west build -p auto -b nrf52840dk_nrf52840 first-example-app
```

## Flash
```bash
west flash
```

## Debug with RTT
```bash
pyrtt-viewer -s [SEGGER_ID] -c 0
# SEGGER_ID can be retrieved with nrfjprog --ids
```
## Applications

Applications in this repo and their description.

### First Example App
Two threads, one spawned during compilation and one dynamically during runtime. Two semaphores used to control which of the threads will be running. When they run, an LED representing that thread will light up and it will print a debugging message.