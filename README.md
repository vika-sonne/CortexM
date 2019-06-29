# ARM Cortex-M rountines
This repository collected the well proved routines implementation.

Is aimed mainly in event-driven and/or single thread environment with variable frequency of the system ticks (power consumption management).
And, of course, code reuse techniques by С++.

Tested in [GNU Arm](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) C++11 toolchain.

## Services/Timer
Timer callback infrastracture.

This implementation not needs common header file to declare the timers array. Linker collect information about timers. This technique has key advantage of static linkage and automatic declaration of timers array. Count of timers exactly as needs.

Memory model has two pieces: ROM (timers callback table) and RAM (timers statuses table).

Runtime conseption includes system tick handler that calls callbacks according to ROM and RAM tables.

## Services/IService
The electronic device Services infrastracture.

Business logic often needs in wide opportunities for the performance of the algorithms. So, Service is event-driven system with system wide state of components that compound the system.

Usually, Service represent a block from functional scheme. And has relationships with other Services that consume and produce system-wide events according with hardware processing of concrete Service.
