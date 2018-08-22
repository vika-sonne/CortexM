
#include "Services/Timer.h"
#include <cstring>

namespace Timer
{
	void Init()
	{
		memset(&_Timers_StatesTable_Begin, 0, &_Timers_StatesTable_End - &_Timers_StatesTable_Begin);
	}

	void Start(uint interval, TimerStateStruct *state, bool restart)
	{
		if(state != NULL && (restart || !state->Enabled))
		{
			state->Interval = interval;
			state->TimeStamp = SystemTime + interval;
			state->Enabled = true;
		}
	}

	void Tick()
	{
		for(auto table = (TimerTableStruct*)&_Timers_Table_Begin; table < (TimerTableStruct*)&_Timers_Table_End; table++)
		{
			auto state = table->State;
			if(state->Enabled && state->TimeStamp <= SystemTime)
			{
				state->TimeStamp = SystemTime + state->Interval;
				table->Callback();
			}
		}
	}
}
