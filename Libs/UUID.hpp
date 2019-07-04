/*
 * Universally unique identifier (UUID) is a 128-bit number used to identify information in computer systems
 * @version 1
 * @author Victoria Danchenko
 * @date 15/06/2011
 */

#ifndef UUID_HPP_
#define UUID_HPP_

#include <cstdint>

namespace System
{

//! @addtogroup Lib
//! @{
//! @addtogroup UUID
//! @{
//! @addtogroup UUID_Exported_Types
//! @{

	struct UUID
	{
		uint8_t Bytes[16];
		static bool IsEqual(const UUID * UUID1, const UUID * UUID2);
	} __attribute__((packed));

//! @}
//! @}
//! @}
}

#endif /* UUID_HPP_ */
