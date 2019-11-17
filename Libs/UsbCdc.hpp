/**
 * UsbCdc.hpp
 *
 * @date 08/11/2013
 * @author Viktoria Danchenko
 */

#ifndef Usb_Cdc_HPP_
#define Usb_Cdc_HPP_

#include "Libs/UsbBase.hpp"

namespace Usb
{
	//! USB CDC (Communications Device Class)
	class Cdc : public UsbBase
	{
	public:

		//! Requests (Abstract Control Model)
		//! @note USB CDC specification, table 4
		enum class RequestsEnum
		{
			SEND_ENCAPSULATED_COMMAND, //!< Issues a command in the format of the supported control protocol
			GET_ENCAPSULATED_RESPONSE, //!< Requests a response in the format of the supported control protocol
			SET_COMM_FEATURE = 0x02, //!< Controls the settings for a particular communication feature
			GET_COMM_FEATURE = 0x03, //!< Returns the current settings for the communication feature
			CLEAR_COMM_FEATURE = 0x04, //!< Clears the settings for a particular communication feature
			SET_LINE_CODING = 0x20, //!< Configures DTE rate, stop-bits, parity, and number-of-character bits
			GET_LINE_CODING = 0x21, //!< Requests current DTE rate, stop-bits, parity, and number-of-character bits
			SET_CONTROL_LINE_STATE = 0x22,//!< RS-232 signal used to tell the DCE device the DTE device is now present
			SEND_BREAK = 0x23, //!< Sends special carrier modulation used to specify RS-232 style break
		};

		//! USB CDC specification, table 50
		struct _PACKED LineCodingStruct
		{
			uint32_t dwDTERate; //!< Data terminal rate, in bits per second
			uint8_t bCharFormat;//!< Stop bits: 0 - 1 Stop bit; 1 - 1.5 Stop bits; 2 - 2 Stop bits
			uint8_t bParityType;//!< Parity: 0 - None; 1 - Odd; 2 - Even; 3 - Mark; 4 - Space
			uint8_t bDataBits; //!< Data bits: 5, 6, 7, 8 or 16
		};

		// UsbBase implementation

		bool setupNonStandartRequest(EndpointStatusStruct* ep, DataPointerStruct *data)
		{
			// non standard SETUP request arrived // process as CDC request

			if((ActiveSetupRequest.bmRequestType & ~(uint8_t)RequestTypeEnum::DIRECTION_DEVICE_TO_HOST)
					!= ((uint8_t)RequestTypeEnum::TYPE_CLASS | (uint8_t)RequestTypeEnum::RECIPIENT_INTERFACE))
				return false;

			switch(ActiveSetupRequest.bRequest)
			{
				case (uint8_t)RequestsEnum::GET_LINE_CODING:
					return data->set((uint8_t *)getLineCoding(), sizeof(LineCodingStruct));

				case (uint8_t)RequestsEnum::SET_LINE_CODING:
					// check request data
					if(uint16_le_get(&ActiveSetupRequest.wLength) != sizeof(LineCodingStruct) || data->Len != sizeof(LineCodingStruct))
						return false;
					setLineCoding((LineCodingStruct *)data->Data);
					break;

				case (uint8_t)RequestsEnum::SET_CONTROL_LINE_STATE:
					setControlLineState(uint16_le_get(&ActiveSetupRequest.wValue));
					break;

				default:
					return false;
			}

			return true;
		}

		// Cdc implementation

		/**
		 * This request allows the host to specify typical asynchronous line-character formatting properties, which may be required by some applications.
		 * This request applies to asynchronous byte stream data class interfaces and endpoints; it also applies to data transfers both from the host to the device and from the device to the host.
		 * @note USB CDC specification, section 6.2.12.
		 */
		virtual void setLineCoding(const LineCodingStruct *lineCoding) = 0;

		/**
		 * This request allows the host to find out the currently configured line coding.
		 * @note USB CDC specification, section 6.2.13.
		 */
		virtual LineCodingStruct *getLineCoding() = 0;

		/**
		 * This request generates RS-232/V.24 style control signals.
		 * @param controlLineState	USB CDC specification, table 51:
		 * - D15..D2
		 * RESERVED (Reset to zero)
		 * - D1
		 * Carrier control for half duplex modems: 0 - Deactivate carrier; 1 - Activate carrier.
		 * This signal corresponds to V.24 signal 105 and RS-232 signal RTS.
		 * The device ignores the value of this bit when operating in full duplex mode.
		 * - D0
		 * Indicates to DCE if DTE is present or not: 0 - Not Present; 1 - Present.
		 * This signal corresponds to V.24 signal 108/2 and RS-232 signal DTR.
		 * @note USB CDC specification, section 6.2.14.
		 */
		virtual void setControlLineState(const uint16_t controlLineState) = 0;
	};

} /* namespace Usb */

#endif /* Usb_Cdc_HPP_ */
