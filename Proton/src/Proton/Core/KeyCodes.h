#pragma once

namespace Proton 
{
	using KeyCode = uint16_t;

	namespace Key
	{
		enum : KeyCode
		{
			//Windows keys
			A					= 0x41,
			B					= 0x42,
			C					= 0x43,
			D					= 0x44,
			E					= 0x45,
			F					= 0x46,
			G					= 0x47,
			H					= 0x48,
			I					= 0x49,
			J					= 0x4A,
			K					= 0x4B,
			L					= 0x4C,
			M					= 0x4D,
			N					= 0x4E,
			O					= 0x4F,
			P					= 0x50,
			Q					= 0x51,
			R					= 0x52,
			S					= 0x53,
			T					= 0x54,
			U					= 0x55,
			V					= 0x56,
			W					= 0x57,
			X					= 0x58,
			Y					= 0x59,
			Z					= 0x5A,

			NUM_0					= 0x30,	/* 0 */
			NUM_1					= 0x31,	/* 1 */
			NUM_2					= 0x32,	/* 2 */
			NUM_3					= 0x33,	/* 3 */
			NUM_4					= 0x34,	/* 4 */
			NUM_5					= 0x35,	/* 5 */
			NUM_6					= 0x36,	/* 6 */
			NUM_7					= 0x37,	/* 7 */
			NUM_8					= 0x38,	/* 8 */
			NUM_9					= 0x39,	/* 9 */

			Left				= 0x25,
			Up					= 0x26,
			Right				= 0x27,
			Down				= 0x28,
			PageUp				= 0x21,
			PageDown			= 0x22,
			End					= 0x23,
			Home				= 0x24,

			Comma				= 0xBC,
			Period				= 0xBE,
			ForwardSlash		= 0xBF,
			BackwardSlash		= 0xDC,
			Equals				= 0xBB,
			Minus				= 0xBD,

			Backspace			= 0x08,
			Tab					= 0x09,
			Clear				= 0x0C,
			Enter				= 0x0D,
			Shift				= 0x10,
			Control				= 0x11,
			Alt					= 0x12,
			Pause				= 0x13,
			CapsLock			= 0x14,
			Space				= 0x20,

			Select				= 0x29,
			Print				= 0x2A,
			Execute				= 0x2B,
			PrintScreen			= 0x2C,
			Insert				= 0x2D,
			Delete				= 0x2E,
			Help				= 0x2F,
			Escape				= 0x1B,

			LeftShift			= 0xA0,
			RightShift			= 0xA1,
			LeftControl			= 0xA2,
			RightControl		= 0xA3,
			LMenu				= 0xA4,
			RMenu				= 0xA5,

			/*Keypad*/
			KP0					= 0x60,	
			KP1					= 0x61,	
			KP2					= 0x62,	
			KP3					= 0x63,	
			KP4					= 0x64,	
			KP5					= 0x65,	
			KP6					= 0x66,	
			KP7					= 0x67,	
			KP8					= 0x68,	
			KP9					= 0x69,	
								
			Multiply			= 0x6A,
			Add					= 0x6B,
			Separator			= 0x6C,
			Subtract			= 0x6D,
			Decimal				= 0x6E,
			Divide				= 0x6F,

			/* Function Keys */
			F1					= 0x70,
			F2					= 0x71,
			F3					= 0x72,
			F4					= 0x73,
			F5					= 0x74,
			F6					= 0x75,
			F7					= 0x76,
			F8					= 0x77,
			F9					= 0x78,
			F10					= 0x79,
			F11					= 0x7A,
			F12					= 0x7B,
			F13					= 0x7C,
			F14					= 0x7D,
			F15					= 0x7E,
			F16					= 0x7F,
			F17					= 0x80,
			F18					= 0x81,
			F19					= 0x82,
			F20					= 0x83,
			F21					= 0x84,
			F22					= 0x85,
			F23					= 0x86,
			F24					= 0x87,	
		};
	}
}