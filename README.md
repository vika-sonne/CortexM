# ARM Cortex-M rountines
This repository collected the well proved routines  of B2B & M2M infrastructure implementation of electronics device side.

Is aimed mainly in event-driven and/or single thread environment with variable frequency of the system ticks (power consumption management).
And, of course, code reuse techniques by С++.
This routines can be used in synchronous algorithms with multithreaded RTOS like [scmRTOS](https://github.com/scmrtos/scmrtos) (that can really unlock the potential of C++).

Tested in [GNU Arm](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) C++11 toolchain.

## Intro

Modern development of electronic devices requires integration into the data aquisition & control infrastructure. So, the embedded developer must have wide knowledge. And be better if several developers will be involved in development process. But how to realize the collaboration along development process. There is solution: provide the system of pieces like a puzzle.

![](/images/block-diagram.png)

C++ is one the sane system-forming tool for bare-bone systems. Of course, with strictly approach according to restrictions of the tool chain (compiler) and hardware abilities:

![](/images/block-diagram2.png)

Almost all needs by business logic & hardware implementation satisfied by next opportunities: timers, system-wide events and algorithms with external API.

## Services/Timer
Timer callback infrastracture.

This implementation not needs common header file to declare the timers array. Linker collect information about timers. This technique has key advantage of static linkage and automatic declaration of timers array. Count of timers exactly as needs.

Memory model has two pieces: ROM (timers callback table) and RAM (timers statuses table):

![](/images/timer.png)

Runtime conseption includes system tick handler that calls callbacks according to ROM and RAM tables:

![](/images/timer2.png)

## Services/IService
The electronic device Services infrastracture.

Business logic often needs in wide opportunities for the performance of the algorithms. So, Service is event-driven system with system wide state of components that compound the system.

Usually, Service represent a block from functional scheme. And has relationships with other Services that consume and produce system-wide events according with hardware processing of concrete Service.

## Libs/PersistentStorage
File system for M2M infrastructure.

## Libs/PageCacheClass
Data cache as memory buffer for page by page access basis. This is part of filesystem with FLASH storage devices and used to achieve the provided lifetime.

Holds one page in RAM buffer while write process and flush the buffer when write to another page happends. Has *PreFlushCallbackStruct* to finalize page data before flush (write). This can be used in the deferred CRC calculation mechanism and for another actions for page data finalization.

![](/images/page-cache.png)
