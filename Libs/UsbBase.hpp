/**
 * USBbase.hpp
 *
 * @date 08/11/2013
 * @author Viktoria Danchenko
 */

#ifndef Usb_UsbBase_HPP_
#define Usb_UsbBase_HPP_

#include <stdint.h>
#include <algorithm>
#include "Libs/BytesOrder.h"

#ifndef _PACKED
#	define _PACKED __attribute__ ((__packed__))
#endif

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define __USB_PLACE_NUM(num)	((num) & 0xFF), (((num) >> 8) & 0xFF)

/**
 * USB specification, table 9-8. Standard Device Descriptor
 *
 * __bcd_usb__
 * 2 bytes
 * USB Specification Release Number in Binary-Coded Decimal (i.e., 2.10 is 210H).
 * This field identifies the release of the USB Specification with which the device and its descriptors are compliant.
 *
 * __device_class__
 * 1 byte
 * Class code (assigned by the USB-IF).
 * If this field is reset to zero, each interface within a configuration specifies its own class information and the various interfaces operate independently.
 * If this field is set to a value between 1 and FEH, the device supports different class specifications on different interfaces and the interfaces may not operate independently.
 * This value identifies the class definition used for the aggregate interfaces.
 * If this field is set to FFH, the device class is vendor-specific.
 *
 * __device_subclass__
 * 1 byte
 * Subclass code (assigned by the USB-IF).
 * These codes are qualified by the value of the bDeviceClassfield.
 * If the bDeviceClassfield is reset to zero, this field must also be reset to zero.
 * If the bDeviceClassfield is not set to FFH, all values are reserved for assignment by the USB-IF.
 *
 * __device_protocol__
 * 1 byte
 * Protocol code (assigned by the USB-IF).
 * These codes are qualified by the value of the bDeviceClass and the bDeviceSubClassfields.
 * If a device supports class-specific protocols on a device basis as opposed to an interface basis, this code identifies the protocols that the device uses as defined by the specification of the device class.
 * If this field is reset to zero, the device does not use class-specific protocols on a device basis. However, it may use class-specific protocols on an interface basis.
 * If this field is set to FFH, the device uses a vendor-specific protocol on a device basis.
 *
 * __max_packet_size__
 * 1 byte
 * Maximum packet size for endpoint zero (only 8, 16, 32, or 64 are valid)
 *
 * __id_vendor__
 * 2 bytes
 * Vendor ID (assigned by the USB-IF)
 *
 * __id_product__
 * 2 bytes
 * Product ID (assigned by the manufacturer)
 *
 * __bcd_device__
 * 2 bytes
 * Device release number in binary-coded decimal
 *
 * __i_manufacturer__
 * 1 byte
 * Index of string descriptor describing manufacturer
 *
 * __i_product__
 * 1 byte
 * Index of string descriptor describing product
 *
 * __i_serial_number__
 * 1 byte
 * Index of string descriptor describing the device�s serial number
 *
 * __num_configurations__
 * 1 byte
 * Number of possible configurations
 */

#define USB_DEVICE_DESCRIPTOR_Declare(__bcd_usb__, __device_class__, __device_subclass__, __device_protocol__, __max_packet_size__, __id_vendor__, __id_product__, __bcd_device__, __i_manufacturer__, __i_product__, __i_serial_number__, __num_configurations__)\
	0x12,\
	USB_DEVICE_DESCRIPTOR_TYPE,\
	__USB_PLACE_NUM(__bcd_usb__),\
	__device_class__,\
	__device_subclass__,\
	__device_protocol__,\
	__max_packet_size__,\
	__USB_PLACE_NUM(__id_vendor__),\
	__USB_PLACE_NUM(__id_product__),\
	__USB_PLACE_NUM(__bcd_device__),\
	__i_manufacturer__,\
	__i_product__,\
	__i_serial_number__,\
	__num_configurations__,

/**
 * USB specification, table 9-10. Standard Configuration Descriptor
 *
 * __num_interfaces__
 * 1 byte
 * Number of interfaces supported by this configuration
 *
 * __configuration_value__
 * 1 byte
 * Value to use as an argument to the setConfiguration() request to select this configuration
 *
 * __i_configuration__
 * 1 byte
 * Index of string descriptor describing this configuration
 *
 * __attributes__
 * 1 byte
 * Configuration characteristics
 * 	D7: Reserved (set to one)
 * 	D6: Self-powered
 * 	D5: Remote Wakeup
 * 	D4...0: Reserved (reset to zero)
 * 	D7 is reserved and must be set to one for historical reasons.
 * A device configuration that uses power from the bus and a local source reports a non-zero value in bMaxPowerto indicate the amount of bus power required and sets D6.
 * The actual power source at runtime may be determined using the GetStatus(DEVICE) request (see Section 9.4.5).
 * If a device configuration supports remote wakeup, D5 is set to one.
 *
 * __max_power__
 * 1 byte
 * Maximum power consumption of the USB device from the bus in this specific configuration when the device is fully operational.
 * Expressed in 2 mA units (i.e., 50 = 100 mA).
 * Note: A device configuration reports whether the configuration is bus-powered or self-powered.
 * Device status reports whether the device is currently self-powered.
 * If a device is disconnected from its external power source, it updates device status to indicate that it is no longer self-powered.
 * A device may not increase its power draw from the bus, when it loses its external power source, beyond the amount reported by its configuration.
 * If a device can continue to operate when disconnected from its external power source, it continues to do so.
 * If the device cannot continue to operate, it fails operations it can no longer support.
 * The USB System Software may determine the cause of the failure by checking the status and noting the loss of the device�s power source.
 */

#define USB_CONFIGURATION_DESCRIPTOR_Declare(__num_interfaces__, __configuration_value__, __i_configuration__, __attributes__, __max_power__, __configuration_data__...)\
	0x09,\
	USB_CONFIGURATION_DESCRIPTOR_TYPE,\
	__USB_PLACE_NUM(sizeof((uint8_t[]){__configuration_data__}) + 9),\
	__num_interfaces__,\
	__configuration_value__,\
	__i_configuration__,\
	__attributes__,\
	__max_power__,\
	__configuration_data__

/**
 * USB specification, table 9-12. Standard Interface Descriptor
 *
 * __interface_number__
 * 1 byte
 * Number Number of this interface.
 * Zero-based value identifying the index in the array of concurrent interfaces supported by this configuration.
 *
 * __alternate_setting__
 * 1 byte
 * Number Value used to select this alternate setting for the interface identified in the prior field
 *
 * __num_endpoints__
 * 1 byte
 * Number Number of endpoints used by this interface (excluding endpoint zero).
 * If this value is zero, this interface only uses the Default Control Pipe.
 *
 * __interface_class__
 * 1 byte
 * Class code (assigned by the USB-IF).
 * A value of zero is reserved for future standardization.
 * If this field is set to FFH, the interface class is vendor-specific.
 * All other values are reserved for assignment by the USB-IF.
 *
 * __interface_subclass__
 * 1 byte
 * Subclass code (assigned by the USB-IF).
 * These codes are qualified by the value of the bInterfaceClassfield.
 * If the bInterfaceClassfield is reset to zero, this field must also be reset to zero.
 * If the bInterfaceClassfield is not set to FFH, all values are reserved for assignment by the USB-IF.
 *
 * __interface_protocol__
 * 1 byte
 * Protocol code (assigned by the USB).
 * These codes are qualified by the value of the bInterfaceClass and the bInterfaceSubClassfields.
 * If an interface supports class-specific requests, this code identifies the protocols that the device uses as defined by the specification of the device class.
 * If this field is reset to zero, the device does not use a class-specific protocol on this interface.
 * If this field is set to FFH, the device uses a vendor-specific protocol for this interface.
 *
 * __i_interface__
 * 1 byte
 * Index of string descriptor describing this interface
 */

#define USB_INTERFACE_DESCRIPTOR_Declare(__interface_number__, __alternate_setting__, __num_endpoints__, __interface_class__, __interface_subclass__, __interface_protocol__, __i_interface__)\
	0x09,\
	USB_INTERFACE_DESCRIPTOR_TYPE,\
	__interface_number__,\
	__alternate_setting__,\
	__num_endpoints__,\
	__interface_class__,\
	__interface_subclass__,\
	__interface_protocol__,\
	__i_interface__,

/**
 * USB specification, table 9-13. Standard Endpoint Descriptor
 *
 * __endpoint_address__
 * 1 byte
 * The address of the endpoint on the USB device described by this descriptor.
 * The address is encoded as follows:
 * 	Bit 3...0: The endpoint number
 * 	Bit 6...4: Reserved, reset to zero
 * 	Bit 7: Direction, ignored for control endpoints
 * 		0 = OUT endpoint
 * 		1 = IN endpoint
 *
 * __attributes__
 * 1 byte
 * This field describes the endpoint�s attributes when it is configured using the bConfigurationValue.
 * 	Bits 1..0: Transfer Type
 * 		00 = Control
 * 		01 = Isochronous
 * 		10 = Bulk
 * 		11 = Interrupt
 * If not an isochronous endpoint, bits 5..2 are reserved and must be set to zero. If isochronous, they are defined as follows:
 *	Bits 3..2: Synchronization Type
 *		00 = No Synchronization
 *		01 = Asynchronous
 *		10 = Adaptive
 *		11 = Synchronous
 *	Bits 5..4: Usage Type
 *		00 = Data endpoint
 *		01 = Feedback endpoint
 *		10 = Implicit feedback Data endpoint
 *		11 = Reserved
 *	Refer to Chapter 5 for more information. All other bits are reserved and must be reset to zero. Reserved bits must be ignored by the host.
 *
 * __max_packet_size__
 * 2 bytes
 * Maximum packet size this endpoint is capable of sending or receiving when this configuration is selected.
 * For isochronous endpoints, this value is used to reserve the bus time in the schedule, required for the per-(micro)frame data payloads. The pipe may, on an ongoing basis, actually use less bandwidth than that reserved. The device reports, if necessary, the actual bandwidth used via its normal, non-USB defined mechanisms.
 * For all endpoints, bits 10..0 specify the maximum packet size (in bytes).
 * For high-speed isochronous and interrupt endpoints:
 * 	Bits 12..11 specify the number of additional transaction opportunities per microframe:
 * 		00 = None (1 transaction per microframe)
 * 		01 = 1 additional (2 per microframe)
 * 		10 = 2 additional (3 per microframe)
 * 		11 = Reserved
 * 	Bits 15..13 are reserved and must be set to zero. Refer to Chapter 5 for more information.
 *
 * __interval__
 * 1 byte
 * Interval for polling endpoint for data transfers.
 * Expressed in frames or microframes depending on the device operating speed (i.e., either 1 millisecond or 125 �s units).
 * For full-/high-speed isochronous endpoints, this value must be in the range from 1 to 16. The bIntervalvalue is used as the exponent for a 2 bInterval-1 value; e.g., a bIntervalof 4 means a period of 8.
 * For full-/low-speed interrupt endpoints, the value of this field may be from 1 to 255.
 * For high-speed interrupt endpoints, thebIntervalvalue is used as the exponent for a 2 ^ (bInterval-1) value; e.g., a bIntervalof 4 means a period of 8.
 * This value must be from 1 to 16.
 * For high-speed bulk/control OUT endpoints, the bIntervalmust specify the maximum NAK rate of the endpoint.
 * A value of 0 indicates the endpoint never NAKs. Other values indicate at most 1 NAK each bIntervalnumber of microframes. This value must be in the range from 0 to 255.
 * See Chapter 5 description of periods for more detail.
 */

#define USB_ENDPOINT_DESCRIPTOR_Declare(__endpoint_address__, __attributes__, __max_packet_size__, __interval__)\
	0x07,\
	USB_ENDPOINT_DESCRIPTOR_TYPE,\
	__endpoint_address__,\
	__attributes__,\
	__USB_PLACE_NUM(__max_packet_size__),\
	__interval__,

/**
 * USB specification, table 9-15. String Descriptor Zero, Specifying Languages Supported by the Device
 */

#define USB_STRING_DESCRIPTOR_Declare(__string__...)\
	sizeof((uint8_t[]){__string__}) + 2,\
	USB_STRING_DESCRIPTOR_TYPE,\
	__string__,

namespace Usb
{
	enum class StandardRequestsEnum
	{
		GET_STATUS = 0,   //!< GET_STATUS
		CLEAR_FEATURE,    //!< CLEAR_FEATURE
		RESERVED1,        //!< RESERVED1
		SET_FEATURE,      //!< SET_FEATURE
		RESERVED2,        //!< RESERVED2
		SET_ADDRESS,      //!< SET_ADDRESS
		GET_DESCRIPTOR,   //!< GET_DESCRIPTOR
		SET_DESCRIPTOR,   //!< SET_DESCRIPTOR
		GET_CONFIGURATION,//!< GET_CONFIGURATION
		SET_CONFIGURATION,//!< SET_CONFIGURATION
		GET_INTERFACE,    //!< GET_INTERFACE
		SET_INTERFACE,    //!< SET_INTERFACE
		SYNCH_FRAME,      //!< SYNCH_FRAME
	};

	enum class DescriptorTypesEnum
	{
		DEVICE = 1,
		CONFIG,
		STRING,
		INTERFACE,
		ENDPOINT,
	};

	//! bmRequestType bits
	enum class RequestTypeEnum
	{
		RECIPIENT_DEVICE,
		RECIPIENT_INTERFACE,
		RECIPIENT_ENDPOINT,
		RECIPIENT_OTHER,

		TYPE_STANDART = 0,
		TYPE_CLASS = 0x20,
		TYPE_VENDOR = 0x40,

		DIRECTION_DEVICE_TO_HOST = 0x80,
	};

	//! USB connection state
	enum class StateEnum
	{
		UNCONNECTED,
		ATTACHED,
		POWERED,
		SUSPENDED,
		ADDRESSED,
		CONFIGURED
	};

	//! The state machine states of a control pipe
	enum class EndpointStateEnum
	{
		WAIT_SETUP,
		IN_DATA,
		IN_DATA_FULL_PACKET,
		IN_DATA_EMPTY_PACKET,
		WAIT_STATUS_OUT,
	};

	//! USB specification, section 9.3. USB Device Requests
	struct _PACKED DeviceRequestStruct
	{
		uint8_t bmRequestType;
		uint8_t bRequest;
		uint16_le_t wValue;
		uint16_le_t wIndex;
		uint16_le_t wLength;
	};

	struct EndpointStatusStruct
	{
		uint8_t Index;	//!< Endpoint index: 0..
		EndpointStateEnum State;
	};

	//! Data piece pointer
	struct DataPointerStruct
	{
		uint8_t *Data;	//!< Pointer to data
		uint Len;		//!< Data length, bytes

		DataPointerStruct() : Data(NULL), Len(0) {}

		DataPointerStruct(uint8_t *data, uint len) : Data(data), Len(len) {}

		inline const DataPointerStruct& operator=(const DataPointerStruct *data)
		{
			DataPointerStruct d(*data);
			return d;
		}

		inline const DataPointerStruct& operator+(const uint offset)
		{
			if(this->Len >= offset)
			{
				DataPointerStruct data(NULL, 0);
				return data;
			}
			DataPointerStruct data(this->Data + offset, this->Len - offset);
			return data;
		}

		inline const bool set(const uint8_t *data, const uint len)
		{
			if(data != NULL && len != 0)
			{
				Data = (uint8_t *)data;
				Len = len;
				return true;
			}
			clear();
			return false;
		}

		inline const void reduceLen(const uint maxLen)
		{
			if(Len > maxLen)
				Len = maxLen;
		}

		inline const bool hasData()
		{
			return Len > 0 && Data != NULL;
		}

		inline const void clear()
		{
			Data = NULL;
			Len = 0;
		}
	};

	class UsbBase
	{
	public:

		//! Active SETUP request
		DeviceRequestStruct ActiveSetupRequest;

		// UsbBase virtual declaration/implementation

		virtual void stateChanged(StateEnum newState);

		/**
		 * Start of frame
		 * @note This procedure intended to call from IRQ
		 */
		virtual void sof() = 0;

		/**
		 * suspended
		 * @note This procedure intended to call from IRQ
		 */
		virtual void suspended();

		/**
		 * Wake up
		 * @note This procedure intended to call from IRQ
		 */
		virtual void wakeUp();

		/**
		 * reset
		 * @note This procedure intended to call from IRQ
		 */
		virtual void reset();

		/**
		 * Gets maximum packet size for endpoint
		 * @param epIndex		Endpoint index
		 * @return Size, bytes
		 */
		virtual uint16_t getMaxPacketSize(uint8_t epIndex) = 0;

		/**
		 * Setup request arrived
		 * @param ep		IN	Endpoint
		 * @param data		IN	Request data
		 * @return True - valid request; false - unsupported request
		 */
		virtual bool setupRequest(EndpointStatusStruct *ep, const DataPointerStruct *data);

		/**
		 * Setup request as non standard request (bmRequestType: class, vendor or reserved)
		 * @param ep		IN		Endpoint
		 * @param data		IN, OUT	Request data IN & answer data OUT
		 * @return True - valid request; false - unsupported request
		 */
		virtual bool setupNonStandartRequest(EndpointStatusStruct* ep, DataPointerStruct *data) = 0;

		/**
		 * Gets device descriptor
		 * @param	data	OUT	Descriptor data
		 * @return True - success
		 */
		virtual bool getDeviceDescriptor(DataPointerStruct *data) = 0;

		/**
		 * Gets config descriptor
		 * @param	data	OUT	Descriptor data
		 * @return True - success
		 */
		virtual bool getConfigDescriptor(DataPointerStruct *data) = 0;

		/**
		 * Gets string descriptor according to USB specification, chapter 9.6.7
		 * @param index		IN	String index: 0 - LANGID codes
		 * @param langId	IN	String LANGID
		 * @param	data	OUT	Descriptor data
		 * @return True - success
		 */
		virtual bool getStringDescriptor(const uint8_t index, const uint16_t langId, DataPointerStruct *data) = 0;

		/**
		 * Sets configuration according to the device configuration descriptor (while SET_CONFIGURATION request. USB specification, section 9.4.7)
		 * @param value		Configuration value
		 * @return True - success; false - value not according to the configuration descriptor
		 */
		virtual bool setConfiguration(uint8_t value) = 0;

	protected:

		uint8_t Current_Configuration;		//!< Selected configuration
		uint8_t Current_Interface;			//!< Selected interface of current configuration
		uint8_t Current_AlternateSetting;	//!< Selected Alternate Setting of current interface
		uint8_t DeviceAddress;				//!< Device address

		DataPointerStruct _setupData; //!< Non-standard request data

		StateEnum _state; //!< USB connection state

		void setState(StateEnum state);

		/**
		 * Control endpoint (EP0) outgoing data (IN packet)
		 * @param ep		IN	Endpoint
		 * @param data		OUT	Data to transmit to host
		 * @note This procedure intended to call from IRQ
		 * @return True - has data to send
		 */
		bool controlEPOutgoingData(EndpointStatusStruct *ep, DataPointerStruct *data);
	};

} /* namespace Usb */

#endif /* Usb_UsbBase_HPP_ */
