/**
* @brief GPIO pin manipulation class template for STM32.
*
* @author 2009-2012Anton Gusev aka AHTOXA (HTTP://AHTOXA.NET)
* @author Inspired by AVR macros from Askold Volkov
* @author Cortex-M0+ and ScmRTOS port by Victoria Danchenko
*
* @copyright Public domain, AS IS.
*
* USAGE:
* @code
* #include "stm32l0xx.h"
* #include "pin.h"
*
* // I. Declare pin typedef:
* typedef Pin<'A', 5, 'H'> PA5;    // PA5, active level = high
* typedef Pin<'A', 6> PA6;         // PA6, active level = high ('H' is default parameter)
* typedef Pin<'B', 12, 'L'> PB12;  // PB12, active level = low
*
* // II. Set pin mode:
* PA5::Config(GPIO::Output_PP_VeryLow);     // configure PA5 as output (push-pull, very low speed)
* PA6::Config(GPIO::Input_Floating);        // configure PA6 as input floating (use object and "." notation)
* PB12::Config(GPIO::Output_PP_VeryLow);    // configure PB12 as output
* TX::Config(GPIO::AF_PP_High);             // configure TX as alternate output high speed push-pull
*                                           // (see GPIO::ConfigEnum for the list of all pin modes)
*
* // III. Manipulate pin:
* PA5::On();               // switch PA5 to active state (H)
* PB12::On();              // switch PB12 to active state (L)
* PA5::Off();              // switch PA5 to inactive state (L)
* PB12::Cpl();             // invert PB12 output
*
* // IV. Check pin state:
* if (PA5::Signalled())     // returns non-zero if pin input = active state (H for PA5)
* if (PB12::Latched())      // returns non-zero if pin output = active state (L for PB12)
*
* // V. Use pin registers uniformly:
* locked = PA5::GPIOx->LCKR & PA5::mask; // read PA5 lock state.
* // or
* locked = ((GPIO_TypeDef*)PA5::GPIOx_BASE)->LCKR & PA5::mask; // read PA5 lock state.
*
* // It is also possible to declare object of defined type:
* PA5 PinA5;
* // In this case, one can use "dot" notation to call object functions, i.e.
* PinA5.On();
* PB12.Config(GPIO::Input_Floating);
* // Note: using objects instead of types can (in some cases) increase memory consumption.
* @endcode
*
*/

#ifndef PIN_H_INCLUDED
#define PIN_H_INCLUDED
#include <stddef.h>
#include <sys/types.h>

#define GPIO_MAKE_PIN_CFG(__MODE_, __OTYPE_, __OSPEED_, __PUPD_)   ((__MODE_) << 5 | (__OTYPE_) << 4 | (__OSPEED_) << 2 | (__PUPD_))
#define GPIO_GET_PIN_CFG_MODE(__VALUE_)    (__VALUE_ >> 5)
#define GPIO_GET_PIN_CFG_OTYPE(__VALUE_)   ((__VALUE_ >> 4) & 1)
#define GPIO_GET_PIN_CFG_OSPEED(__VALUE_)  ((__VALUE_ >> 2) & 3)
#define GPIO_GET_PIN_CFG_PUPD(__VALUE_)    (__VALUE_ & 3)
#define GPIO_IS_ALTERNATE_MODE(__VALUE_)   (GPIO_GET_PIN_CFG_MODE(__VALUE_) == 2)
#define GPIO_IS_OUTPUT_MODE(__VALUE_)      (GPIO_GET_PIN_CFG_MODE(__VALUE_) == 1)

namespace GPIO
{
	//! Port bit configuration table
	//! @note PP = push-pull, PU = pull-up, PD = pull-down, OD = open-drain, AF = alternate function
	enum ConfigEnum
	{
		Output_PP_VeryLow = 	GPIO_MAKE_PIN_CFG(1, 0, 0, 0),
		Output_PP_Low = 		GPIO_MAKE_PIN_CFG(1, 0, 1, 0),
		Output_PP_Medium = 		GPIO_MAKE_PIN_CFG(1, 0, 2, 0),
		Output_PP_High = 		GPIO_MAKE_PIN_CFG(1, 0, 3, 0),

		Output_PP_PU_VeryLow =	GPIO_MAKE_PIN_CFG(1, 0, 0, 1),
		Output_PP_PU_Low =		GPIO_MAKE_PIN_CFG(1, 0, 1, 1),
		Output_PP_PU_Medium =	GPIO_MAKE_PIN_CFG(1, 0, 2, 1),
		Output_PP_PU_High =		GPIO_MAKE_PIN_CFG(1, 0, 3, 1),

		Output_PP_PD_VeryLow =	GPIO_MAKE_PIN_CFG(1, 0, 0, 2),
		Output_PP_PD_Low =		GPIO_MAKE_PIN_CFG(1, 0, 1, 2),
		Output_PP_PD_Medium =	GPIO_MAKE_PIN_CFG(1, 0, 2, 2),
		Output_PP_PD_High =		GPIO_MAKE_PIN_CFG(1, 0, 3, 2),

		Output_OD_VeryLow = 	GPIO_MAKE_PIN_CFG(1, 1, 0, 0),
		Output_OD_Low = 		GPIO_MAKE_PIN_CFG(1, 1, 1, 0),
		Output_OD_Medium = 		GPIO_MAKE_PIN_CFG(1, 1, 2, 0),
		Output_OD_High = 		GPIO_MAKE_PIN_CFG(1, 1, 3, 0),

		Output_OD_PU_VeryLow =	GPIO_MAKE_PIN_CFG(1, 1, 0, 1),
		Output_OD_PU_Low =		GPIO_MAKE_PIN_CFG(1, 1, 1, 1),
		Output_OD_PU_Medium =	GPIO_MAKE_PIN_CFG(1, 1, 2, 1),
		Output_OD_PU_High =		GPIO_MAKE_PIN_CFG(1, 1, 3, 1),

		Output_OD_PD_VeryLow =	GPIO_MAKE_PIN_CFG(1, 1, 0, 2),
		Output_OD_PD_Low =		GPIO_MAKE_PIN_CFG(1, 1, 1, 2),
		Output_OD_PD_Medium =	GPIO_MAKE_PIN_CFG(1, 1, 2, 2),
		Output_OD_PD_High =		GPIO_MAKE_PIN_CFG(1, 1, 3, 2),

		AF_PP_VeryLow = 		GPIO_MAKE_PIN_CFG(2, 0, 0, 0),
		AF_PP_Low = 			GPIO_MAKE_PIN_CFG(2, 0, 1, 0),
		AF_PP_Medium = 			GPIO_MAKE_PIN_CFG(2, 0, 2, 0),
		AF_PP_High = 			GPIO_MAKE_PIN_CFG(2, 0, 3, 0),

		AF_PP_PU_VeryLow =		GPIO_MAKE_PIN_CFG(2, 0, 0, 1),
		AF_PP_PU_Low =			GPIO_MAKE_PIN_CFG(2, 0, 1, 1),
		AF_PP_PU_Medium =		GPIO_MAKE_PIN_CFG(2, 0, 2, 1),
		AF_PP_PU_High =			GPIO_MAKE_PIN_CFG(2, 0, 3, 1),

		AF_PP_PD_VeryLow =		GPIO_MAKE_PIN_CFG(2, 0, 0, 2),
		AF_PP_PD_Low =			GPIO_MAKE_PIN_CFG(2, 0, 1, 2),
		AF_PP_PD_Medium =		GPIO_MAKE_PIN_CFG(2, 0, 2, 2),
		AF_PP_PD_High =			GPIO_MAKE_PIN_CFG(2, 0, 3, 2),

		AF_OD_VeryLow = 		GPIO_MAKE_PIN_CFG(2, 1, 0, 0),
		AF_OD_Low = 			GPIO_MAKE_PIN_CFG(2, 1, 1, 0),
		AF_OD_Medium = 			GPIO_MAKE_PIN_CFG(2, 1, 2, 0),
		AF_OD_High = 			GPIO_MAKE_PIN_CFG(2, 1, 3, 0),

		AF_OD_PU_VeryLow =		GPIO_MAKE_PIN_CFG(2, 1, 0, 1),
		AF_OD_PU_Low =			GPIO_MAKE_PIN_CFG(2, 1, 1, 1),
		AF_OD_PU_Medium =		GPIO_MAKE_PIN_CFG(2, 1, 2, 1),
		AF_OD_PU_High =			GPIO_MAKE_PIN_CFG(2, 1, 3, 1),

		AF_OD_PD_VeryLow =		GPIO_MAKE_PIN_CFG(2, 1, 0, 2),
		AF_OD_PD_Low =			GPIO_MAKE_PIN_CFG(2, 1, 1, 2),
		AF_OD_PD_Medium =		GPIO_MAKE_PIN_CFG(2, 1, 2, 2),
		AF_OD_PD_High =			GPIO_MAKE_PIN_CFG(2, 1, 3, 2),

		Input_Floating = 		GPIO_MAKE_PIN_CFG(0, 0, 0, 0),
		Input_PU = 				GPIO_MAKE_PIN_CFG(0, 0, 0, 1),
		Input_PD = 				GPIO_MAKE_PIN_CFG(0, 0, 0, 2),
		Analog = 				GPIO_MAKE_PIN_CFG(3, 0, 0, 0),
	};
}

template<char port> struct port_gpio_t;

#ifdef GPIOA_BASE
template<> struct port_gpio_t<'A'>
{
	enum { GPIOx_BASE = GPIOA_BASE };
};
#endif

#ifdef GPIOB_BASE
template<> struct port_gpio_t<'B'>
{
	enum { GPIOx_BASE = GPIOB_BASE };
};
#endif

#ifdef GPIOC_BASE
template<> struct port_gpio_t<'C'>
{
	enum { GPIOx_BASE = GPIOC_BASE };
};
#endif

#ifdef GPIOD_BASE
template<> struct port_gpio_t<'D'>
{
	enum { GPIOx_BASE = GPIOD_BASE };
};
#endif

#ifdef GPIOE_BASE
template<> struct port_gpio_t<'E'>
{
	enum { GPIOx_BASE = GPIOE_BASE };
};
#endif

#ifdef GPIOF_BASE
template<> struct port_gpio_t<'F'>
{
	enum { GPIOx_BASE = GPIOF_BASE };
};
#endif

#ifdef GPIOG_BASE
template<> struct port_gpio_t<'G'>
{
	enum { GPIOx_BASE = GPIOG_BASE };
};
#endif

static void _GPIO_Set_Pull_State(GPIO_TypeDef* gpio_base, uint32_t pin, uint32_t pupdy)
{
	uint32_t buff;
#ifdef scmRTOS_VERSION
	TCritSect cs;
#endif
	buff = gpio_base->PUPDR;
	buff &= ~(3UL << (pin * 2));
	buff |= pupdy << (pin * 2);
	gpio_base->PUPDR = buff;
}

template<char port, int pin_no, char activestate = 'H'> struct Pin;

template<char port, int pin_no, char activestate>
struct Pin
{
	enum { GPIOx_BASE = port_gpio_t<port>::GPIOx_BASE };
	static struct
	{
		GPIO_TypeDef* operator-> () { return (GPIO_TypeDef*)GPIOx_BASE; }
	} GPIOx;

	static const int pin = pin_no;
	static const int port_no = port - 'A';
	static const uint32_t mask = 1UL << pin_no;

	static void On()
	{
		activestate == 'L' ? ((GPIO_TypeDef*)GPIOx_BASE)->BRR = mask : ((GPIO_TypeDef*)GPIOx_BASE)->BSRR = mask;
	}
	static void Off()
	{
		activestate == 'L' ? ((GPIO_TypeDef*)GPIOx_BASE)->BSRR = mask : ((GPIO_TypeDef*)GPIOx_BASE)->BRR = mask;
	}

	//! Inverses the output register bit value
	static void Cpl()
	{
#ifdef scmRTOS_VERSION
		TCritSect cs;
#endif
		((GPIO_TypeDef*)GPIOx_BASE)->BSRR = mask << (((((GPIO_TypeDef*)GPIOx_BASE)->IDR & mask) != 0) ? 16 : 0);
	}

	/**
	 * Configures the GPIO pin
	 * @param config	Config enum
	 * @param value		GPIO pin mode depended. Output mode: set pin state: 0 - Off; 1 - On; else - don't change. Alternate mode: alternate function selection: 0..
	 * @note Thread safe
	 */
	static void Config(GPIO::ConfigEnum config, uint value = 0)
	{
		uint32_t buff;

		if(GPIO_IS_ALTERNATE_MODE(config))
		{
			// alternate function selection for GPIO pin multiplexer
			buff = ((GPIO_TypeDef*)GPIOx_BASE)->AFR[pin / 8];
			buff &= ~(0xF << ((pin % 8) * 4));
			buff |= value << ((pin % 8) * 4);
			((GPIO_TypeDef*)GPIOx_BASE)->AFR[pin / 8] = buff;
		}
		else if(GPIO_IS_OUTPUT_MODE(config))
		{
			// set the output pin state
			switch(value)
			{
				case 0: Off(); break;
				case 1: On(); break;
				default: break;
			}
		}

		{
#ifdef scmRTOS_VERSION
			TCritSect cs;
#endif
			buff = ((GPIO_TypeDef*)GPIOx_BASE)->MODER;
			buff &= ~(3UL << (pin * 2));
			buff |= GPIO_GET_PIN_CFG_MODE(config) << (pin * 2);
			((GPIO_TypeDef*)GPIOx_BASE)->MODER = buff;
		}
		{
#ifdef scmRTOS_VERSION
			TCritSect cs;
#endif
			buff = ((GPIO_TypeDef*)GPIOx_BASE)->OTYPER;
			buff &= ~mask;
			buff |= GPIO_GET_PIN_CFG_OTYPE(config) << pin;
			((GPIO_TypeDef*)GPIOx_BASE)->OTYPER = (uint16_t)buff;
		}
		{
#ifdef scmRTOS_VERSION
			TCritSect cs;
#endif
			buff = ((GPIO_TypeDef*)GPIOx_BASE)->OSPEEDR;
			buff &= ~(3UL << (pin * 2));
			buff |= GPIO_GET_PIN_CFG_OSPEED(config) << (pin * 2);
			((GPIO_TypeDef*)GPIOx_BASE)->OSPEEDR = buff;
		}
		_GPIO_Set_Pull_State((GPIO_TypeDef*)GPIOx_BASE, pin, GPIO_GET_PIN_CFG_PUPD(config));
	}

	//! Set the pin pull-up state
	//! @param onPullUp		State: true - pull-up; false - no pull-up, no pull-down
	static inline void PullUp(bool onPullUp = true)
	{
		_GPIO_Set_Pull_State((GPIO_TypeDef*)GPIOx_BASE, pin, onPullUp ? 1 : 0);
	}

	//! Set the pin pull-down state
	//! @param onPullUp		State: true - pull-down; false - no pull-up, no pull-down
	static inline void PullDown(bool onPullDown = true)
	{
		_GPIO_Set_Pull_State((GPIO_TypeDef*)GPIOx_BASE, pin, onPullDown ? 2 : 0);
	}

	//! Reads the output register bit value
	static int Latched()
	{
		int ret = (((GPIO_TypeDef*)GPIOx_BASE)->ODR & mask) != 0;
		return activestate == 'L' ? !ret : ret;
	}

	//! Reads the input register bit value
	static int Signalled()
	{
		int ret = (((GPIO_TypeDef*)GPIOx_BASE)->IDR & mask) != 0;
		return activestate == 'L' ? !ret : ret;
	}

};

#undef GPIO_MAKE_PIN_CFG
#undef GPIO_GET_PIN_CFG_MODE
#undef GPIO_GET_PIN_CFG_OSPEED
#undef GPIO_GET_PIN_CFG_OTYPE
#undef GPIO_GET_PIN_CFG_PUPD
#undef GPIO_IS_ALTERNATE_MODE
#undef GPIO_IS_OUTPUT_MODE

#endif // PIN_H_INCLUDED
