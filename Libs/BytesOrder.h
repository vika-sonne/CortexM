/**
 * Bytes-order conversion module
 *
 * @date 10/10/2012
 * @author Viktoria Danchenko
 */

#ifdef __cplusplus
#include <stdint.h>
extern "C" {
#endif

	typedef unsigned int uint;

	//! Little-endian
	typedef struct uint16_le_t
	{
		uint8_t Bytes[2];
	} uint16_le_t;

	//! Little-endian
	typedef struct uint32_le_t
	{
		uint8_t Bytes[4];
	} uint32_le_t;

	/**
	 * Gets value from little-endian
	 * @param d		Little-endian to convert
	 * @return Converted value
	 * @note (d) is alignment insensible
	 */
	inline uint16_t uint16_le_get(uint16_le_t* d)
	{
		return (d->Bytes[1] << 8) + d->Bytes[0];
	}

	/**
	 * Gets value from little-endian
	 * @param d		Little-endian to convert
	 * @return Converted value
	 * @note (d) is alignment insensible
	 */
	inline uint32_t uint32_le_get(uint32_le_t* d)
	{
		return (d->Bytes[3] << 24) + (d->Bytes[2] << 16) + (d->Bytes[1] << 8) + d->Bytes[0];
	}

	//! big-endian
	typedef struct uint16_be_t
	{
		uint8_t Bytes[2];
	} uint16_be_t;

	//! big-endian
	typedef struct uint32_be_t
	{
		uint8_t Bytes[4];
	} uint32_be_t;

	/**
	 * Gets value from big-endian
	 * @param d		Big-endian to convert
	 * @return Converted value
	 * @note (d) is alignment insensible
	 */
	inline uint16_t uint16_be_get(uint16_be_t* d)
	{
		return (d->Bytes[0] << 8) + d->Bytes[1];
	}

	/**
	 * Gets value from big-endian
	 * @param d		Big-endian to convert
	 * @return Converted value
	 * @note (d) is alignment insensible
	 */
	inline uint32_t uint32_be_get(uint32_be_t* d)
	{
		return (d->Bytes[0] << 24) + (d->Bytes[1] << 16) + (d->Bytes[2] << 8) + d->Bytes[3];
	}

#ifdef __cplusplus
}
#endif
