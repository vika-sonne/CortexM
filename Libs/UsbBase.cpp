#include <string.h>
#include "Libs/USBbase.hpp"

namespace Usb
{
	bool UsbBase::setupRequest(EndpointStatusStruct *ep, const DataPointerStruct *data)
	{
		// standard SETUP request arrived // check the request
		if(data->Len < sizeof(DeviceRequestStruct))
			return false;

		// save the SETUP request
		memcpy(&ActiveSetupRequest, data->Data, sizeof(ActiveSetupRequest));

		// process according to the standard SETUP request
		// check request code - USB specification, table 9-4
		switch(ActiveSetupRequest.bRequest)
		{
			case StandardRequestsEnum::GET_DESCRIPTOR:
				// USB specification, chapter 9.4.3
				if(ActiveSetupRequest.bmRequestType != RequestTypeEnum::DIRECTION_DEVICE_TO_HOST)
					return false;
				// check descriptor type - USB specification, table 9-5
				switch(ActiveSetupRequest.wValue.Bytes[1])
				{
					case DescriptorTypesEnum::DEVICE:
						if(!getDeviceDescriptor(&_setupData))
							return false;
						break;
					case DescriptorTypesEnum::CONFIG:
						if(!getConfigDescriptor(&_setupData))
							return false;
						break;
					case DescriptorTypesEnum::STRING:
					{
						if(!getStringDescriptor(ActiveSetupRequest.wValue.Bytes[0], uint16_le_get(&ActiveSetupRequest.wIndex), &_setupData))
							return false;
						break;
					}
				}
				// check for answer length limit
				_setupData.reduceLen(uint16_le_get(&ActiveSetupRequest.wLength));
				break;

			case StandardRequestsEnum::SET_ADDRESS:
				if(ActiveSetupRequest.bmRequestType != 0)
					return false;
				if(_state < StateEnum::ADDRESSED)
				{
					DeviceAddress = ActiveSetupRequest.wValue.Bytes[0] & 0x7F;
					setState(StateEnum::ADDRESSED);
				}
				else
					return false;
				break;

			case StandardRequestsEnum::SET_CONFIGURATION:
				if(ActiveSetupRequest.bmRequestType != 0)
					return false;	// not follow USB specification, table 9-3
				// check state // allowed for ADDRESSED & CONFIGURED only
				if(_state < StateEnum::ADDRESSED)
					return false; // not allowed
				if(_state == StateEnum::CONFIGURED && ActiveSetupRequest.wValue.Bytes[0] == 0)
					setState(StateEnum::ADDRESSED);
				else
				{
					// set configuration according to the device configuration descriptor
					if(!setConfiguration(ActiveSetupRequest.wValue.Bytes[0]))
						return false;
					Current_Configuration = ActiveSetupRequest.wValue.Bytes[0];
					// change state
					switch(_state)
					{
						case StateEnum::ADDRESSED:
							setState(StateEnum::CONFIGURED);
							break;
						case StateEnum::CONFIGURED:
							setState(StateEnum::ADDRESSED);
							setState(StateEnum::CONFIGURED);
							break;
						default:
							return false;
					}
				}
				break;

			case StandardRequestsEnum::GET_STATUS:
				break;

			case StandardRequestsEnum::GET_CONFIGURATION:
				break;

			case StandardRequestsEnum::GET_INTERFACE:
				break;

			default:
				// non-standard request

				_setupData = data + sizeof(DeviceRequestStruct);
				if(!setupNonStandartRequest(ep, &_setupData))
				{
					_setupData.clear();
					return false;
				}

				// check for answer length limit
				_setupData.reduceLen(uint16_le_get(&ActiveSetupRequest.wLength));
				break;
		}

		return true;
	}

	bool UsbBase::controlEPOutgoingData(EndpointStatusStruct *ep, DataPointerStruct *data)
	{
		if(_setupData.hasData())
		{
			// send answer for SETUP request
			uint16_t nextPacketSize = std::min<uint>(_setupData.Len, getMaxPacketSize(ep->Index));
			memcpy(data, &_setupData, sizeof(_setupData));
			data->reduceLen(nextPacketSize);
			_setupData = _setupData + nextPacketSize;
			return true;
		}

		return false;	// no data to send
	}

	void UsbBase::reset()
	{
		setState(StateEnum::UNCONNECTED);
		setState(StateEnum::ATTACHED);
	}

	void UsbBase::suspended()
	{
		setState(StateEnum::SUSPENDED);
	}

	void UsbBase::wakeUp()
	{
		setState(Current_Configuration != 0 ? StateEnum::CONFIGURED : StateEnum::ATTACHED);
	}

	void UsbBase::setState(StateEnum state)
	{
		if(_state != state)
		{
			stateChanged(state);
			_state = state;
		}
	}

	void UsbBase::stateChanged(StateEnum newState)
	{
		switch(_state)
		{
			case StateEnum::UNCONNECTED:
				_setupData.clear();
			case StateEnum::ATTACHED:
				Current_Configuration = Current_Interface = Current_AlternateSetting = DeviceAddress = 0;
				memset(&ActiveSetupRequest, 0, sizeof(ActiveSetupRequest));
				break;
			default:
				break;
		}
	}
}
