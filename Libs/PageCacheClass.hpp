/**
 * Data cache as memory buffer for page by page access basis
 * @version 2
 * @author Victoria Danchenko
 * @date 21/06/2018
 * 
 * @note Main purpose is translate memory access from linear address space (user) to page basis storage
 * with RAM cache of pages.
 * This can help to reduce count of erase/write to storage (e.g. FLASH).
 * And simplify storage API by support page basis operations only.
 * And build more complex storage system that including cache pool, thanks to C++ code reuse feature.
 * There is advanced control of flushing page process (@see PreFlushCallbackType).
 */

#ifndef SRC_LIB_PAGECACHECLASS_HPP_
#define SRC_LIB_PAGECACHECLASS_HPP_

#include <algorithm>

namespace System
{
	namespace Cache
	{
		//! Cache as memory buffer for page by page access basis
		template <typename ADDRESS_TYPE, unsigned int PAGE_SIZE>
		class PageCacheClass
		{
		public:
			enum class StatusEnum { Empty, HasData, Dirty };

			//! @param buffer	Cache buffer (page)
			//! @param len		Cache buffer length, bytes
			//! @param address	Address of storage device to store a cache buffer
			//! @param arg		Misc argument
			typedef void (*PreFlushCallbackType)(const void *buffer, unsigned int len, ADDRESS_TYPE address, void *arg);

			//! Used to callback before flush the cache
			struct PreFlushCallbackStruct
			{
				PreFlushCallbackType Callback;
				void *arg;
				PreFlushCallbackStruct(PreFlushCallbackType callback, void *arg) : Callback(callback), arg(arg) {}
			};

		protected:
			ADDRESS_TYPE m_Address; //!< Start address of cache buffer
			StatusEnum m_Status = StatusEnum::Empty;
			char m_Buffer[PAGE_SIZE]; //!< Cache buffer

			//! Write from a cache buffer
			//! @param buffer	Cache buffer to write from
			//! @param len		Cache buffer length, bytes: @c PAGE_SIZE
			//! @param address	Start address of page to write to; it's always @c PAGE_SIZE aligned
			//! @note This function is called for entire page size & page address align
			virtual bool Write(const void *buffer, ADDRESS_TYPE address, unsigned int len)=0;

			//! Read to a cache buffer
			//! @param buffer	Cache buffer to read to
			//! @param len		Read length, bytes: @c 1..PAGE_SIZE
			//! @param address	Start address of page to read from
			//! @none This function is not @c PAGE_SIZE aligned
			virtual bool Read(void *buffer, ADDRESS_TYPE address, unsigned int len)=0;

			bool setAddress(ADDRESS_TYPE address, PreFlushCallbackStruct *callback=nullptr)
			{
				if(m_Status == StatusEnum::Dirty && !isInCache(address))
				{
					// has data to flush
					if(!Flush(callback))
						return false;
				}
				m_Address = address & ~((ADDRESS_TYPE)PAGE_SIZE-1);
				return true;
			}

		public:

			//! Returns cache status
			inline StatusEnum getStatus() const { return m_Status; }

			//! Returns start address of cache (first byte of cached page)
			ADDRESS_TYPE getAddress() const { return m_Address; }

			//! Checks, is address in cache buffer
			inline bool isInCache(ADDRESS_TYPE address) const { return m_Status != StatusEnum::Empty && m_Address == (address & ~((ADDRESS_TYPE)PAGE_SIZE-1)); }

			//! Clears the cache buffer (reset)
			inline void Clear() { m_Status = StatusEnum::Empty; }

			//! Flush cache buffer
			//! @param callback	Used to callback before flush the cache; callback happens in case of dirty status only
			bool Flush(PreFlushCallbackStruct *callback=nullptr)
			{
				if(m_Status == StatusEnum::Dirty)
				{
					if(callback != nullptr && callback->Callback != nullptr)
						callback->Callback(m_Buffer, m_Address, sizeof(m_Buffer), callback->arg);
					if(!Write(m_Buffer, m_Address, sizeof(m_Buffer)))
						return false;
					Clear();
				}
				return true;
			}

			//! Writes data; random access: data can be within many pages & unaligned
			//! @param data		Buffer to read to
			//! @param address	Address to read
			//! @param len		Read length, bytes
			//! @param callback	Used to callback before flush the cache; callback happens in case of dirty status only
			//! @note This procedure can cause a series of @c Flush calls
			bool SetData(const void *data, ADDRESS_TYPE address, unsigned int len, PreFlushCallbackStruct *callback=nullptr)
			{
				do
				{
					if((address % PAGE_SIZE) == 0 && len >= PAGE_SIZE)
					{
						// data is entire page // just write data, keep the cache buffer unchanged
						if(!Write(data, address, sizeof(m_Buffer)))
							return false;
						data = (char*)data + sizeof(m_Buffer);
						address += sizeof(m_Buffer);
						len -= sizeof(m_Buffer);
					}
					else
					{
						// data is page unaligned // use cache buffer
						if(!setAddress(address, callback))
							return false;
						// if data start address is not aligned by page border, then read the unaligned data piece at the beginning of page
						if(m_Status == StatusEnum::Empty && m_Address != address && !Read(m_Buffer, m_Address, address % PAGE_SIZE))
							return false;
						auto restPageSize = PAGE_SIZE - (address % PAGE_SIZE);
						auto pageDataLen = std::min<decltype(len)>(restPageSize, len); // current page data length
						// copy the rest page data to buffer
						memcpy(&m_Buffer[address % PAGE_SIZE], data, pageDataLen);
						// if data piece is not aligned by page border, then read the unaligned data piece at the ending of page
						if(m_Status == StatusEnum::Empty && restPageSize > pageDataLen && !Read(&m_Buffer[(address % PAGE_SIZE) + pageDataLen], m_Address + (address % PAGE_SIZE) + pageDataLen, restPageSize - pageDataLen))
							return false;
						m_Status = StatusEnum::Dirty;
						data = (char*)data + pageDataLen;
						address += pageDataLen;
						len -= pageDataLen;
					}
				} while(len > 0);
				return true;
			}

			//! Reads data; random access: data can be within many pages & unaligned
			//! @param data		Buffer to read to
			//! @param address	Address to read
			//! @param len		Read length, bytes
			bool GetData(void *data, ADDRESS_TYPE address, unsigned int len)
			{
				do
				{
					auto restPageSize = PAGE_SIZE - (address % PAGE_SIZE);
					auto pageDataLen = std::min<decltype(len)>(restPageSize, len); // current page data length
					if(isInCache(address))
						memcpy(data, &m_Buffer[address % PAGE_SIZE], pageDataLen);
					else if(!Read(data, address, pageDataLen))
						return false;
					data = (char*)data + pageDataLen;
					address += pageDataLen;
					len -= pageDataLen;
				} while(len > 0);
				return true;
			}
		};
	}
}

#endif /* SRC_LIB_PAGECACHECLASS_HPP_ */
