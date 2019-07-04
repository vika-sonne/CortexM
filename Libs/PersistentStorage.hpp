/**
 * Data persistent storage. Used to maintain data on the FLASH memory.
 * @version 3
 * @author Victoria Danchenko
 * @date 15/06/2018
 * 
 * @note This class is base for FLASH storage working algorithms.
 * Main purpose is translate memory access from linear address space (user) to FLASH storage pages address space basis.
 * The persistent storage workflow includes the set of user datas that has identification based on UUID.
 * Storage consistency based on pages chain identification by UUID and each page integrity by CRC.
 * The page based CRC let adopt to FLASH hardware controller peculiarity and rewrite only
 * the pages according to changed user data piece without CRC recalculation of another pages while writing process.
 * Of course, re-writing changed pages can be optimized by leaving this pages untouched
 * and new data write in another pages according to write balancing algorithm of the FLASH pages pool.
 * So, the persistent storage is a FLASH pages pool where one page is cortege of user data (payload)
 * and auxiliary information (@see StorageHeaderStruct).
 */

#ifndef SRC_LIB_PERSISTENTSTORAGE_HPP_
#define SRC_LIB_PERSISTENTSTORAGE_HPP_

#include "UUID.hpp"
#include <string.h>

namespace System
{
	namespace PersistentStorage
	{
		//! Header of the storage
		//! @note Used to identify and check the storage & user data
		template <typename ADDRESS_TYPE, typename CRC_TYPE>
		struct StorageHeaderStruct
		{
			System::UUID Uuid;		//!< UUID of the persistent storage (magic)
			System::UUID DataUuid;	//!< UUID of user data. Used to identify raw data
			ADDRESS_TYPE Length;	//!< Length of the persistent storage (excluding storage header, client data only), bytes
			CRC_TYPE StorageCrc;	//!< CRC of the persistent storage (excluding header)
		} __attribute__((packed));

		static const System::UUID StorageUUID = { 0xB0, 0x24, 0xF2, 0xDC, 0x72, 0xEA, 0x11, 0xE8, 0x85, 0x8E, 0x2C, 0xFD, 0xA1, 0xE1, 0xCE, 0xF5 };

		template <typename ADDRESS_TYPE, typename CRC_TYPE>
		class StorageReaderClass
		{
		public:
			enum class StorageCheckEnum { Ok, NoStorage, AnotherStorage, DeviceError, StorageError };
		protected:

			//! Compare device data with pattern
			//! @param pattern	Data pattern to compare
			//! @param address	Address into device data space, bytes
			//! @param len		Data length to compare
			virtual bool Compare(const void *pattern, ADDRESS_TYPE address, unsigned int len) const=0;

			//! Calculate CRC for device
			//! @param address	Address into device data space, bytes
			//! @param len		Data length to calculate, bytes
			virtual CRC_TYPE CalculateCRC(ADDRESS_TYPE address, unsigned int len) const=0;

			//! Read data from device
			//! @param data		Buffer to read to
			//! @param address	Address into device data space, bytes
			//! @param len		Read length, bytes
			virtual bool Read(void *data, ADDRESS_TYPE address, unsigned int len) const=0;

			inline bool getLength(ADDRESS_TYPE address, ADDRESS_TYPE &length) const
			{
				return Read(&length, address + (decltype(address))&(((StorageHeaderStruct<ADDRESS_TYPE, CRC_TYPE>*)0)->Length), sizeof(length));
			}

			inline bool getCrc(ADDRESS_TYPE address, CRC_TYPE &crc) const
			{
				return Read(&crc, address + (decltype(address))&(((StorageHeaderStruct<ADDRESS_TYPE, CRC_TYPE>*)0)->StorageCrc), sizeof(crc));
			}

			ADDRESS_TYPE m_Address; //!< Address into the device space

		public:

			//! @param address	Address into the device space
			inline StorageReaderClass(ADDRESS_TYPE address) : m_Address(address) {}

			//! Checks is persistent storage correct (including storage data)
			//! @param address	Address into device data space (storage address), bytes
			//! @param uuid		UUID of user data
			StorageCheckEnum IsStorageCorrect(ADDRESS_TYPE address, const System::UUID uuid)
			{
				// check storage UUID
				if(!Compare(&StorageUUID, address, sizeof(StorageUUID)))
					return StorageCheckEnum::NoStorage; // wrong storage UUID
				// check persistent data UUID
				if(!Compare(&uuid, address + (decltype(address))&(((StorageHeaderStruct<ADDRESS_TYPE, CRC_TYPE>*)0)->DataUuid), sizeof(uuid)))
					return StorageCheckEnum::AnotherStorage; // wrong data UUID
				// compare CRC from storage header and calculated CRC
				decltype(StorageHeaderStruct<ADDRESS_TYPE, CRC_TYPE>::Length) len;
				decltype(StorageHeaderStruct<ADDRESS_TYPE, CRC_TYPE>::StorageCrc) crc;
				if(!getLength(address, len))
					return StorageCheckEnum::DeviceError; // device error
				if(!getCrc(address, crc))
					return StorageCheckEnum::DeviceError; // device error
				if(crc == CalculateCRC(address + sizeof(StorageHeaderStruct<ADDRESS_TYPE, CRC_TYPE>), len))
				{
					m_Address = address;
					return StorageCheckEnum::Ok;
				}
				return StorageCheckEnum::StorageError;
			}

			//! Gets data from storage
			//! @param data		Buffer to write to
			//! @param len		Buffer length, bytes
			//! @param offset	Storage offset, bytes
			bool GetData(void *data, unsigned int len, unsigned int offset=0) const
			{
				// check out of data bound
				decltype(StorageHeaderStruct<ADDRESS_TYPE, CRC_TYPE>::Length) dataLength;
				if(!getLength(m_Address, dataLength))
					return false; // device error
				if(len + offset > dataLength)
					return false; // out of data bound error
				return Read(data, m_Address, len + offset);
			}
		};

		template <typename ADDRESS_TYPE, typename CRC_TYPE>
		class StorageWriterClass
		{
		protected:

			//! Write data to device
			//! @param data		Buffer to write from
			//! @param len		Write length, bytes
			//! @param address	Address into device data space, bytes
			virtual bool Write(const void *data, unsigned int len, ADDRESS_TYPE address) const=0;

			ADDRESS_TYPE m_Address; //!< Address into the device space
			System::UUID m_Uuid; //!< UUID of persistent data

		public:

			//! @param address	Address into the device space
			inline StorageWriterClass(ADDRESS_TYPE address, const System::UUID uuid) : m_Address(address), m_Uuid(uuid) {}

			//! Sets data to storage
			//! @param data		Buffer to read from
			//! @param len		Buffer length, bytes
			//! @param crc		Storage CRC
			bool SetData(const void *data, unsigned int len, CRC_TYPE crc) const
			{
				// check out of data bound
				if(!Write(&StorageUUID, sizeof(StorageUUID), m_Address))
					return false; // device error
				if(!Write(&m_Uuid, sizeof(m_Uuid), m_Address + (decltype(m_Address))&(((StorageHeaderStruct<ADDRESS_TYPE, CRC_TYPE>*)0)->DataUuid)))
					return false; // device error
				if(!Write(&len, sizeof(len), m_Address + (decltype(m_Address))&(((StorageHeaderStruct<ADDRESS_TYPE, CRC_TYPE>*)0)->Length)))
					return false; // device error
				if(!Write(&crc, sizeof(crc), m_Address + (decltype(m_Address))&(((StorageHeaderStruct<ADDRESS_TYPE, CRC_TYPE>*)0)->StorageCrc)))
					return false; // device error
				if(!Write(data, len, m_Address + sizeof(StorageHeaderStruct<ADDRESS_TYPE, CRC_TYPE>)))
					return false; // device error
				return true;
			}
		};

		static const System::UUID PageStorageUUID = { 0xD2, 0x3C, 0x3B, 0x7A, 0x75, 0xF9, 0x11, 0xE8, 0x81, 0x90, 0x2C, 0xFD, 0xA1, 0xE1, 0xCE, 0xF5 };

		//! Storage using the pages chain
		template <typename ADDRESS_TYPE, typename LENGTH_TYPE, typename CRC_TYPE>
		class PageStorageClass
		{
		public:
			enum class PageCheckResultEnum { Ok, NoStorage, AnotherStorage, DeviceError, Error };
			struct CheckOptions
			{
				bool DontCheckCrc : 1;
				bool DontCheckMetrics : 1;
				CheckOptions(bool dontCheckCrc=false, bool dontCheckMetrics=false) : DontCheckCrc(dontCheckCrc), DontCheckMetrics(dontCheckMetrics) {}
			};
		protected:
			//! Header of the storage page
			//! @note Used to identify and check the storage page & user data
			struct PageHeaderStruct
			{
				System::UUID Uuid; //!< UUID of the persistent storage (magic). Used to identify the storage
				System::UUID DataUuid; //!< UUID of user data. Used to identify the user data
				LENGTH_TYPE TotalLength; //!< Length of the user data of all pages into the chain (excluding storage headers, user data only), bytes
				LENGTH_TYPE PageOffset; //!< Offset of the user data of current page, bytes
				LENGTH_TYPE PageLength; //!< Length of the user data of current page (excluding page header, user data only), bytes
				CRC_TYPE PageCrc; //!< CRC of the user data (excluding page header, user data only)
			} __attribute__((packed));

			//! Aligned version of @c PageHeaderStruct metrics
			struct PageHeaderMetricsStruct
			{
				LENGTH_TYPE TotalLength; //!< Length of the user data of all pages into the chain (excluding storage headers, user data only), bytes
				LENGTH_TYPE PageOffset; //!< Offset of the user data of current page, bytes
				LENGTH_TYPE PageLength; //!< Length of the user data of current page (excluding page header, user data only), bytes
				CRC_TYPE PageCrc; //!< CRC of the user data (excluding page header, user data only)
			};

			const System::UUID &m_Uuid; //!< UUID of user data
			ADDRESS_TYPE m_Address; //!< Address into the storage device space

			//! Compare storage device data with pattern
			//! @param pattern	Data pattern to compare
			//! @param address	Address into device data space, bytes
			//! @param len		Data length to compare
			virtual bool Compare(const void *pattern, ADDRESS_TYPE address, LENGTH_TYPE len) const=0;

			//! Read data from storage device
			//! @param data		Buffer to read to
			//! @param address	Address into device data space, bytes
			//! @param len		Read length, bytes
			virtual bool Read(void *data, ADDRESS_TYPE address, LENGTH_TYPE len) const=0;

			//! Calculate CRC for storage device data
			//! @param address	Address into device data space, bytes
			//! @param len		Data length to calculate, bytes
			virtual CRC_TYPE CalculatePageCRC(ADDRESS_TYPE address, LENGTH_TYPE len) const=0;

			//! Write within one page to storage device
			//! @param data		Buffer to write from
			//! @param address	Address of page start into device data space, bytes
			virtual bool WritePage(const void *data, ADDRESS_TYPE address, LENGTH_TYPE len) const=0;

			inline bool getMetrics(PageHeaderMetricsStruct &metrics) const { return getMetrics(metrics, m_Address); }

			bool getMetrics(PageHeaderMetricsStruct &metrics, ADDRESS_TYPE address) const
			{
				if(!Read(&metrics.TotalLength, address + (decltype(address))&(((PageHeaderStruct*)0)->TotalLength), sizeof(PageHeaderStruct::TotalLength)))
					return false;
				if(!Read(&metrics.PageOffset, address + (decltype(address))&(((PageHeaderStruct*)0)->PageOffset), sizeof(PageHeaderStruct::PageOffset)))
					return false;
				if(!Read(&metrics.PageLength, address + (decltype(address))&(((PageHeaderStruct*)0)->PageLength), sizeof(PageHeaderStruct::PageLength)))
					return false;
				if(!Read(&metrics.PageCrc, address + (decltype(address))&(((PageHeaderStruct*)0)->PageCrc), sizeof(PageHeaderStruct::PageCrc)))
					return false;
				return true;
			}

			//! Sets storage header
			//! @param data		Buffer to read from
			//! @param len		Buffer length, bytes
			//! @param crc		Storage CRC
			bool SetHeader(const PageHeaderMetricsStruct &metrics) const
			{
				// check out of data bound
				if(!WritePage(&PageStorageUUID, m_Address, sizeof(PageStorageUUID)))
					return false; // device error
				if(!WritePage(&m_Uuid, m_Address + (decltype(m_Address))&(((PageHeaderStruct*)0)->DataUuid), sizeof(m_Uuid)))
					return false; // device error
				if(!WritePage(&metrics.TotalLength, m_Address + (decltype(m_Address))&(((PageHeaderStruct*)0)->TotalLength), sizeof(metrics.TotalLength)))
					return false; // device error
				if(!WritePage(&metrics.PageOffset, m_Address + (decltype(m_Address))&(((PageHeaderStruct*)0)->PageOffset), sizeof(metrics.PageOffset)))
					return false; // device error
				if(!WritePage(&metrics.PageLength, m_Address + (decltype(m_Address))&(((PageHeaderStruct*)0)->PageLength), sizeof(metrics.PageLength)))
					return false; // device error
				if(!WritePage(&metrics.PageCrc, m_Address + (decltype(m_Address))&(((PageHeaderStruct*)0)->PageCrc), sizeof(metrics.PageCrc)))
					return false; // device error
				return true;
			}

			static inline LENGTH_TYPE getMaxPageLength(LENGTH_TYPE pageLen) { return pageLen - sizeof(PageHeaderStruct); }

		public:

			//! @param uuid		UUID of user data
			//! @param address	Address into the storage device space
			PageStorageClass(const System::UUID &uuid, ADDRESS_TYPE address=0) : m_Uuid(uuid), m_Address(address) {}

			//! Checks is page correct (including user data)
			//! @param address		Address into device data space (storage address), bytes
			//! @param pageLen		Page length, bytes: @c sizeof(PageHeaderStruct)..
			PageCheckResultEnum isPageCorrect(ADDRESS_TYPE address, LENGTH_TYPE pageLen, const CheckOptions options=CheckOptions())
			{
				// check storage UUID
				if(!Compare(&PageStorageUUID, address, sizeof(PageStorageUUID)))
					return PageCheckResultEnum::NoStorage; // wrong storage UUID
				// check persistent data UUID
				if(!Compare(&m_Uuid, address + (decltype(address))&(((PageHeaderStruct*)0)->DataUuid), sizeof(m_Uuid)))
					return PageCheckResultEnum::AnotherStorage; // wrong data UUID
				if(!options.DontCheckMetrics)
				{
					// compare CRC from page header and calculated CRC for this page
					PageHeaderMetricsStruct metrics;
					if(!getMetrics(metrics, address))
						return PageCheckResultEnum::DeviceError; // device error
					if(metrics.PageLength > getMaxPageLength(pageLen) || metrics.PageLength > metrics.TotalLength || metrics.PageOffset > metrics.TotalLength)
						return PageCheckResultEnum::Error; // storage error
					if(!options.DontCheckCrc && metrics.PageCrc != CalculatePageCRC(address + sizeof(PageHeaderStruct), metrics.PageLength))
						return PageCheckResultEnum::Error; // CRC error
				}
				m_Address = address;
				return PageCheckResultEnum::Ok;
			}
		};
	}
}

#endif /* SRC_LIB_PERSISTENTSTORAGE_HPP_ */
