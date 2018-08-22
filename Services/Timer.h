/**
 * Event-driven single thread environment timer.
 * Use system tick interrupt call to process two tables: callbacks table (FLASH), status table (RAM).
 * This implementation uses compile time memory allocation. So all the timer callbacks (FLASH) and timer states (RAM) collect by linker into two sections: FLASH and RAM.
 *
 * @file Timer.h
 * @author vika
 * @version 3
 */

/**
 * @page Timer
 * @par Linker script sections:
 * - Timers table (ROM, FLASH)
 * - Timers states table (RAM)
 * @code
SECTIONS
{
	.timers :
	{
		. = ALIGN(4);
		PROVIDE(_Timers_Table_Begin = .);
		KEEP(*(.timers .timers.*))
		PROVIDE(_Timers_Table_End = .);
	} >FLASH
	.bss (NOLOAD) :
	{
	    . = ALIGN(4);
		PROVIDE(_Timers_StatesTable_Begin = .);
		KEEP(*(.timers_states .timers_states.*))
		PROVIDE(_Timers_StatesTable_End = .);
	} >RAM
}
 * @endcode
 * @par Usage
 * Main .cpp file:
 * @code
#include "Services/Timer.h"
int main(int argc, char* argv[])
{
	Timer::Init();
	for(;;)
	{
		Timer::Tick();
	}
}
 * @endcode
 * Example .cpp file:
 * @code
#include "Services/Timer.h"
TIMER_DECLARE(ExampleTimer)
void some_function()
{
	auto timestamp = Timer::Now(); // system time, mS
	Timer::Start(1000, TIMER_STATE(ExampleTimer)); // start the timer with interval; if timer is running - nothing will changed
	Timer::Start(1000, TIMER_STATE(ExampleTimer), true); // set a new timer interval
	auto interval = Timer::Interval(TIMER_STATE(ExampleTimer)); // get timer interval
	auto isStarted = Timer::isStarted(TIMER_STATE(ExampleTimer));
}
TIMER_CALLBACK(BufferTimer)
{
	Timer::Stop(TIMER_STATE(ExampleTimer));
}
 * @endcode
 */

#ifndef SRC_TIMER_H_
#define SRC_TIMER_H_

#include <sys/types.h>

#define TIMER_DECLARE(name)\
	static void _Timer_##name();\
	static Timer::TimerStateStruct _TimerState_##name __attribute__((section(".timers_states"),used));\
	static const Timer::TimerTableStruct __attribute__((section(".timers"),used)) _TimerTable_##name = { &_TimerState_##name, &_Timer_##name }; \

#define TIMER_CALLBACK(name)\
	static void _Timer_##name()

#define TIMER_CALLBACK_CALL(name)\
	_Timer_##name()

#define TIMER_STATE(name) &_TimerState_##name

extern "C"
{
//! @addtogroup groupLinker
//! @{
	extern unsigned char _Timers_Table_Begin; //!< ROM/Flash address, first byte of array
	extern unsigned char _Timers_Table_End; //!< ROM/Flash address, first byte after array
	extern unsigned char _Timers_StatesTable_Begin; //!< RAM address, first byte of array
	extern unsigned char _Timers_StatesTable_End; //!< RAM address, first byte after array
//! @}
//! @defgroup groupTimer Timer
//! @{
	extern volatile uint32_t SystemTime;  //!< System time since epoch (device initialization), mS
}


namespace Timer
{
	typedef void (*TimerCallback)();

	struct TimerStateStruct
	{
		uint32_t Interval;   //!< Callback interval, mS
		uint32_t TimeStamp;  //!< Time of next callback
		bool Enabled;        //!< True - timer is enabled to produce callback; false - timer is disabled
	};

	struct TimerTableStruct
	{
		TimerStateStruct* State;
		TimerCallback Callback;
	};

	//! Starts timer
	//! @param interval		Interval, mS: 0..
	//! @param state		Timer state struct: @b TIMER_STATE(@a<TimerName>)
	//! @param restart		True - restart from now; false - don't restart if timer is enabled
	void Start(uint interval, TimerStateStruct *state, bool restart = false);

	//! Ends timer
	//! @param state		Timer state struct: @b TIMER_STATE(@a<TimerName>)
	inline void Stop(TimerStateStruct *state) { if(state != NULL) state->Enabled = false; }

	//! @param state		Timer state struct: @b TIMER_STATE(@a<TimerName>)
	inline bool isStarted(TimerStateStruct *state) { return state->Enabled; }

	//! @param state		Timer state struct: @b TIMER_STATE(@a<TimerName>)
	inline decltype(TimerStateStruct::Interval) Interval(TimerStateStruct *state) { return state->Interval; }

	//! Returns system time, mS
	static inline decltype(TimerStateStruct::Interval) Now() { return SystemTime; }

	//! Process timers table since the system time was changed
	//! @note Timers callback notification function. Call this usually from the message loop
	void Tick();

	void Init();

} /* namespace Timer */

//! @}

#endif /* SRC_TIMER_H_ */
