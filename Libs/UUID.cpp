/*
 * Universally unique identifier (UUID) is a 128-bit number used to identify information in computer systems
 * @version 1
 * @author Victoria Danchenko
 * @date 15/06/2011
 */

#include "UUID.hpp"
#include <string.h>

namespace System
{

//! @addtogroup Lib
//! @{
//! @addtogroup UUID
//! @{
//! @addtogroup UUID_External_Functions
//! @{

	bool UUID::IsEqual(const UUID * o1, const UUID * o2)
	{
		if(o1 == NULL || o2 == NULL)
			return false;
		return memcmp(o1, o2, sizeof(UUID)) == 0;
	}

//! @}
//! @}
//! @}
}
