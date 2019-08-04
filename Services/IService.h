
/**
 * @page Services
 * @par Config
 * @code
#define SERVICES_STATETYPE
 * @endcode
 * @par Linker script sections:
 * - Services table (ROM, FLASH)
 * - Services states table (RAM)
 * Linker script example:
 * @code
SECTIONS
{
	.services :
	{
		. = ALIGN(4);
		PROVIDE(_Services_Table_Begin = .);
		KEEP(*(.services .services.*))
		PROVIDE(_Services_Table_End = .);
	} >FLASH
	.bss (NOLOAD) :
	{
		. = ALIGN(4);
		PROVIDE(_Services_StatesTable_Begin = .);
		KEEP(*(.services_states .services_states.*))
		PROVIDE(_Services_StatesTable_End = .);
	} >RAM
}
 * @endcode
 * @par Usage
 * Main .cpp file:
 * @code
#include "Services/IService.h"
int main(int argc, char* argv[])
{
	Services::Init();
	for(;;)
	{
		Services::ProcessStates();
	}
}
 * @endcode
 * Example service .h file:
 * @code
namespace Services
{
	namespace Example
	{
		enum class StateEnum { Start = 1, USB_Power = 2, S1_Pressed = 4 , S1_LongPressed = 8 };
		enum class StateLocalEnum { S1 = 1 };
		extern const char *ServiceName;
	}
}
 * @endcode
 * Example service .cpp file:
 * @code
#include "Services/IService.h"
namespace Services
{
	namespace Example
	{
		static bool Enable(bool enable)
		{}
		//! State changed by another service
		static void StateChanged(const char *name, StateType stateBits, StateType changedStateMask)
		{
			if(name == Services::Main::ServiceName)
			{
				if((changedStateMask | (StateType)Services::Main::StateEnum::Start) && (stateBits | (StateType)Services::Main::StateEnum::Start))
				{}
			}
		}
		//! State changed by this service
		static void StateChangedBy(const char *name, StateType &stateBits, StateType changedStateMask)
		{
			if(changedStateMask & (StateType)StateEnum::S1_Pressed)
				stateBits &= ~(StateType)StateEnum::S1_Pressed;
		}
		//! Local state changed by an IRQ of this service
		void LocalStateChanged(const char *name __attribute__((unused)), StateType &stateBits)
		{
			if(stateBits & (StateType)StateLocalEnum::S1)
			{}
			stateBits = 0;
		}
		SERVICE_DECLARE(Example, &Enable, &StateChanged, &StateChangedBy, &LocalStateChanged)
	}
}
extern "C"
{
	void EXTI4_15_IRQHandler()
	{
		// check for S1 button handler
		if(EXTI->PR & EXTI_PR_PIF13)
		{
			EXTI->PR |= EXTI_PR_PIF13; // clear pending interrupt
			Services::SetLocalState(Services::Main::ServiceName, (Services::StateType)Services::Main::StateLocalEnum::S1);
		}
	}
}
 * @endcode
 */

#ifndef SRC_ISERVICE_H_
#define SRC_ISERVICE_H_

#include <stddef.h>
#include <stdint.h>

//! @addtogroup groupLinker
//! @{
extern "C"
{
	extern unsigned char _Services_Table_Begin; //!< ROM/Flash address, first byte of array
	extern unsigned char _Services_Table_End; //!< ROM/Flash address, first byte after array
	extern unsigned char _Services_StatesTable_Begin; //!< RAM address, first byte of array
	extern unsigned char _Services_StatesTable_End; //!< RAM address, first byte after array
}
//! @}

//! @defgroup groupServices Services
//! @{

namespace Services
{
#ifndef SERVICES_STATETYPE
	typedef uint32_t StateType;
#endif

	//! Service state
	struct IServiceStateStruct
	{
		bool Enabled;
		StateType State; //!< State bits
		StateType ChangedState; //!< State bit mask
		StateType LocalChangedState; //!< State bit mask for own service only
//		IServiceStateStruct() :
//			Enabled(false), State(0), ChangedState(0) {}
		inline void Clear()
		{
			State = ChangedState = LocalChangedState = 0;
		}
		inline bool SetState(StateType setBitsMask, bool force = false)
		{
			return SetState(setBitsMask, setBitsMask, force);
		}
		bool SetState(StateType stateBits, StateType stateMask, bool force = false)
		{
			if(!Enabled && !force)
				return false;
			ChangedState |= (State ^ stateBits) & stateMask; // remember really changed bits
//			ChangedState |= stateMask; // remember changed bits
			State &= ~stateMask; // clear state bits
			State |= stateBits; // set appropriate state bits
			return true;
		}
		bool SetLocalState(StateType stateBits, bool force = false)
		{
			if(!Enabled && !force)
				return false;
			LocalChangedState |= stateBits;
			return true;
		}
	};

	//! Enable/disable callback
	//! @param name				Service name. Usually Services::<service_name>::ServiceName
	//! @param enable			True - enable
	//! @return True - success; false - service can't be enabled/disabled
	//! @note For this service return value is processed.
	//! @note For another service return value is ignored.
	typedef bool (*EnableCallback)(const char *name, bool enable);

	//! State of another service was changed
	//! @param name				Service name. Usually Services::<service_name>::ServiceName
	//! @param stateBits		State bits
	//! @param changedStateMask	Mask of changed state bits
	typedef void (*StateChangedCallback)(const char *name, StateType stateBits, StateType changedStateMask);

	//! State of own service was changed. Used to emit pulse kind states to clear pulsed bits after processing by all another services
	//! @param name				Service name. Usually Services::<service_name>::ServiceName
	//! @param stateBits		State bits
	//! @param changedStateMask	Mask of changed state bits
	typedef void (*StateChangedByCallback)(const char *name, StateType &stateBits, StateType changedStateMask);

	//! Local state of own service was changed. Used to emit pulse kind states to own service from ISR e.t.c.
	//! @param name				Service name. Usually Services::<service_name>::ServiceName
	//! @param localStateBits	Local state bits
	typedef void (*LocalStateChangedCallback)(const char *name, StateType &localStateBits);

	struct IServiceTableEntryStruct
	{
		const char *Name;
		const EnableCallback Enable;
		const StateChangedCallback StateChanged;
		const StateChangedByCallback StateChangedBy;
		const LocalStateChangedCallback LocalStateChanged;
//		IServiceTableEntryStruct(const char *name, EnableCallback enableCallback, StateChangedCallback stateChanged, StateChangedByCallback stateChangedBy) :
//			Name(name), Enable(enableCallback), StateChanged(stateChanged), StateChangedBy(stateChangedBy) {}
	};

	//! Enables/disables the service
	//! @param name		Service name. Usually Services::<service_name>::ServiceName. Can be NULL to action thru all services
	//! @param enable	True - enable; false - disable
	//! @return True - success; false - service not found or it can't be enabled/disabled
	bool Enable(const char *name, bool enable = true);

	//! Checks is service enabled
	//! @param name		Service name. Usually Services::<service_name>::ServiceName
	//! @return True - enabled; false - service not enabled or it can't be found
	bool isEnabled(const char *name);

	//! Sets/clears state bits
	//! @param name			Service name. Usually Services::<service_name>::ServiceName
	//! @param stateBits	Bits to set/clear
	//! @param stateMask	Bits mask
	//! @return True - success; false - service not found or it disabled
	bool SetState(const char *name, StateType stateBits, StateType stateMask);

	//! Sets/clears state bits
	//! @param name			Service name. Usually Services::<service_name>::ServiceName
	//! @param stateBits	Bits to set
	//! @return True - success; false - service not found or it disabled
	inline bool SetState(const char *name, StateType stateBits) { return SetState(name, stateBits, stateBits); }

	//! Gets state bits
	//! @param name			Service name. Usually Services::<service_name>::ServiceName
	//! @return State bits
	StateType State(const char *name);

	//! Sets/clears state bits for own service processing
	//! @param name			Service name. Usually Services::<service_name>::ServiceName
	//! @param stateBits	Bits to set
	//! @return True - success; false - service disabled
	bool SetLocalState(const char *name, StateType stateBits);

	void Init();

	//! Services callback notification function. Call this usually from the message loop
	//! @note One call to this function is the one events process round
	void ProcessStates();

}  // namespace Services

#define SERVICE_DECLARE(name, enableCallback, stateChangedCallback, stateChangedByCallback, localStateChangedCallback)\
	const IServiceTableEntryStruct _ServiceTable_##name __attribute__((section(".services"),used)) {#name, enableCallback, stateChangedCallback, stateChangedByCallback, localStateChangedCallback};\
	const char *ServiceName = _ServiceTable_##name.Name;\
	static IServiceStateStruct _ServiceState_##name __attribute__((section(".services_states"),used));

#define SERVICE_STATE(name) _ServiceState_##name

//! @}

#endif /* SRC_ISERVICE_H_ */
