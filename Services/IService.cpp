
#include "IService.h"
#include <cstring>

namespace Services
{
	//! Finds service index into ServiceTable
	//! @return Index: 0..; -1 - service not found
	static int findIndex(const char *name)
	{
		auto i = 0;
		for(auto table = (IServiceTableEntryStruct *)&_Services_Table_Begin; (unsigned char *)table < &_Services_Table_End; table++, i++)
			if(table->Name == name)
				return i;
		return -1;
	}

	void Init()
	{
		memset(&_Services_StatesTable_Begin, 0, &_Services_StatesTable_End - &_Services_StatesTable_Begin);

#ifdef SEGGER_SYSVIEW_H
		auto i = 0;
		for(auto table = (IServiceTableEntryStruct *)&_Services_Table_Begin; (unsigned char *)table < &_Services_Table_End; table++, i++)
		{
			SEGGER_SYSVIEW_OnTaskCreate(SEGGER_SYSVIEW_TaskBase + i);
			{
				SEGGER_SYSVIEW_TASKINFO info = { (uint32_t)(SEGGER_SYSVIEW_TaskBase + i), table->Name, 0, 0, 0 };
				SEGGER_SYSVIEW_SendTaskInfo(&info);
			}
		}
#endif
	}

	bool isEnabled(const char *name)
	{
		auto i = findIndex(name);
		if(i >= 0)
			return ((IServiceStateStruct*)&_Services_StatesTable_Begin)[i].Enabled;
		return false;
	}

	bool Enable(const char *name, bool enable)
	{
		auto i = 0;
		for(auto table = (IServiceTableEntryStruct *)&_Services_Table_Begin; (unsigned char *)table < &_Services_Table_End; table++, i++)
		{
			if(name == NULL || table->Name == name)
			{
				auto state = &((IServiceStateStruct*)&_Services_StatesTable_Begin)[i];
//				if(table->Enable != NULL && state->Enabled != enable && table->Enable(enable))
//					state->Enabled = enable;
				if(table->Enable != NULL && state->Enabled != enable)
				{
#ifdef SEGGER_SYSVIEW_H
					if(enable)
						SEGGER_SYSVIEW_OnTaskStartExec(SEGGER_SYSVIEW_TaskBase + i);
#endif
					if(table->Enable(table->Name, enable))
					{
						// success
						state->Enabled = enable;
						// callback another services
						if(name != NULL)
						{
							for(auto table2 = (IServiceTableEntryStruct *)&_Services_Table_Begin; (unsigned char *)table2 < &_Services_Table_End; table2++)
								if(table2->Name != name && table2->Enable != NULL)
									table2->Enable(name, enable);
						}
					}
#ifdef SEGGER_SYSVIEW_H
					if((state->Enabled == enable && !enable) || (state->Enabled != enable && enable))
						SEGGER_SYSVIEW_OnTaskStopReady(SEGGER_SYSVIEW_TaskBase + i, 0);
#endif
				}
				if(name != NULL)
					return state->Enabled == enable;
			}
		}
		return name == NULL;
	}

	StateType State(const char *name)
	{
		auto i = findIndex(name);
		if(i < 0)
			return (StateType)0;
		return ((IServiceStateStruct*)&_Services_StatesTable_Begin)[i].State;
	}

	bool SetState(const char *name, StateType stateBits, StateType stateMask)
	{
		auto i = findIndex(name);
		if(i < 0)
			return false;
		return ((IServiceStateStruct*)&_Services_StatesTable_Begin)[i].SetState(stateBits, stateMask);
	}

	bool SetLocalState(const char *name, StateType stateBits)
	{
		auto i = findIndex(name);
		if(i < 0)
			return false;
		return ((IServiceStateStruct*)&_Services_StatesTable_Begin)[i].SetLocalState(stateBits);
	}

	void ProcessStates()
	{
		auto i = 0;
		for(auto table = (IServiceTableEntryStruct *)&_Services_Table_Begin; (unsigned char *)table < &_Services_Table_End; table++, i++)
		{
			auto state = &((IServiceStateStruct*)&_Services_StatesTable_Begin)[i];
			if(state->Enabled)
			{
				// state bits processing
				if(state->ChangedState != 0)
				{
					// some state bits was changed since previous process // run callback for all another services
					for(auto table2 = (IServiceTableEntryStruct *)&_Services_Table_Begin; (unsigned char *)table2 < &_Services_Table_End; table2++)
						if(table2->Name != table->Name && table2->StateChanged != NULL)
							table2->StateChanged(table->Name, state->State, state->ChangedState); // is another service & callback is defined
					// run callback for own service
					if(table->StateChangedBy != NULL)
						table->StateChangedBy(table->Name, state->State, state->ChangedState); // own callback is defined
					state->ChangedState = 0;
				}
				// local state bits processing
				if(table->LocalStateChanged != NULL && state->LocalChangedState != 0)
					table->LocalStateChanged(table->Name, state->LocalChangedState);
			}
		}
	}
}
