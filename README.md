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

Example:
```C++
#include "Services/Timer.h"

TIMER_DECLARE(ExampleTimer) // declare the timer holder variable

void some_function()
{
	auto timestamp = Timer::Now(); // system time, mS
	Timer::Start(1000, TIMER_STATE(ExampleTimer)); // start the timer with interval; if timer is running - nothing will changed
	Timer::Start(1000, TIMER_STATE(ExampleTimer), true); // set a new timer interval
	auto interval = Timer::Interval(TIMER_STATE(ExampleTimer)); // get timer interval
	auto isStarted = Timer::isStarted(TIMER_STATE(ExampleTimer));
}

// timer callback function
TIMER_CALLBACK(ExampleTimer)
{
	Timer::Stop(TIMER_STATE(ExampleTimer));
}
```

## Services/IService
The electronic device Services infrastracture.

Business logic often needs in wide opportunities for the performance of the algorithms. So, Service is event-driven system with system wide state of components that compound the system.

Usually, Service represent a block from functional scheme. And has relationships with other Services that consume and produce system-wide events according with hardware processing of concrete Service.

![](/images/services.png)

Example of S1 button service with button IRQ handler:

Example.h file:

```C++
namespace Services
{
	namespace Example
	{
		enum class StateEnum { Start = 1, USB_Power = 2, S1_Pressed = 4 , S1_LongPressed = 8 };
		enum class StateLocalEnum { S1 = 1, Overload = 2 };
		extern const char *ServiceName;
	}
}
```

Example.cpp file:

```C++
#include "Services/IService.h"
#include "Services/main.h" // main service. Used for the system-wide statuses & pulses
#include "Example.h"

namespace Services
{
	namespace Example
	{
		const char *ServiceName = "Example";

		//! Enable/disable the service
		//! @note Called to enable/disable of events process for this service
		static bool Enable(bool enable)
		{
			if(enable)
			{
				// enable the hardware
			}
			else
			{
				// disable the hardware
			}
		}

		//! State changed by another service
		static void StateChanged(const char *name, StateType stateBits, StateType changedStateMask)
		{
			if(name == Services::Main::ServiceName)
			{
				if(changedStateMask | (StateType)Services::Main::StateEnum::Start)
				{
					// main start/stop - the system-wide pulse
					if(stateBits | (StateType)Services::Main::StateEnum::Start)
					{
						// start of the system // start processing of S1 button
					}
					else
					{
						// stop of the system // stop processing of S1 button
					}
				}
			}
		}

		//! State changed by this service
		//! @note Called at end of events process round
		static void StateChangedBy(const char *name, StateType &stateBits, StateType changedStateMask)
		{
			// processing the pulses
			if(changedStateMask & (StateType)StateEnum::S1_Pressed)
				// clear a state bit
				stateBits &= ~(StateType)StateEnum::S1_Pressed;
		}

		//! Local state changed by an IRQ of this service
		void LocalStateChanged(const char *name __attribute__((unused)), StateType &stateBits)
		{
			if(stateBits & (StateType)StateLocalEnum::Overload)
			{
				// overload // previous button press by IRQ handler was not handled
			}

			if(stateBits & (StateType)StateLocalEnum::S1)
				// S1 button pressed by IRQ handler
				SERVICE_STATE(Example).SetState(StateEnum::S1_Pressed)

			stateBits = 0;
		}

		// declare the service holder variables
		SERVICE_DECLARE(Example, &Enable, &StateChanged, &StateChangedBy, &LocalStateChanged)
	}
}

extern "C"
{
	// S1 button handler // for STM32 hardware
	void EXTI4_15_IRQHandler()
	{
		if(EXTI->PR & EXTI_PR_PIF13) // is pending interrupt
		{
			EXTI->PR |= EXTI_PR_PIF13; // clear pending interrupt

			// set the local state from IRQ context

			if(SERVICE_STATE(Example).LocalChangedState & StateLocalEnum::S1)
				// overload // previous button press was not handled
				SERVICE_STATE(Example).SetLocalState((Services::StateType)StateLocalEnum::Overload);

			SERVICE_STATE(Example).SetLocalState((Services::StateType)StateLocalEnum::S1);
		}
	}
}
```

## Libs/PersistentStorage
File system for M2M infrastructure.

## Libs/PageCacheClass
Data cache as memory buffer for page by page access basis. This is part of filesystem with FLASH storage devices and used to achieve the provided lifetime.

Holds one page in RAM buffer while write process and flush the buffer when write to another page happends. Has *PreFlushCallbackStruct* to finalize page data before flush (write). This can be used in the deferred CRC calculation mechanism and for another actions for page data finalization.

![](/images/page-cache.png)

## Libs/Usb*

Class | Description
------|------------
UsbBase | Base class for hardware abstraction from USB specification.
Cdc | USB Class Definitions for Communication Devices. Successor of UsbBase class.
