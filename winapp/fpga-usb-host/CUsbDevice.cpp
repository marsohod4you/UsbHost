#include "stdafx.h"
#include "CUsbDevice.h"
#include "CUsbHost.h"

//USB HID to PS/2 Scan Code Translation Table
uint32_t hid2scan[] =
{
	/* a A           07 */ 0x04, 0x1E, // 9E 1C F0 1C
	/* b B           07 */ 0x05, 0x30, // B0 32 F0 32
	/* c C           07 */ 0x06, 0x2E, // AE 21 F0 21
	/* d D           07 */ 0x07, 0x20, // A0 23 F0 23
	/* e E           07 */ 0x08, 0x12, // 92 24 F0 24
	/* f F           07 */ 0x09, 0x21, // A1 2B F0 2B
	/* g G           07 */ 0x0A, 0x22, // A2 34 F0 34
	/* h H           07 */ 0x0B, 0x23, // A3 33 F0 33
	/* i I           07 */ 0x0C, 0x17, // 97 43 F0 43
	/* j J           07 */ 0x0D, 0x24, // A4 3B F0 3B
	/* k K           07 */ 0x0E, 0x25, // A5 42 F0 42
	/* l L           07 */ 0x0F, 0x26, // A6 4B F0 4B
	/* m M           07 */ 0x10, 0x32, // B2 3A F0 3A
	/* n N           07 */ 0x11, 0x31, // B1 31 F0 31
	/* o O           07 */ 0x12, 0x18, // 98 44 F0 44
	/* p P           07 */ 0x13, 0x19, // 99 4D F0 4D
	/* q Q           07 */ 0x14, 0x10, // 90 15 F0 15
	/* r R           07 */ 0x15, 0x13, // 93 2D F0 2D
	/* s S           07 */ 0x16, 0x1F, // 9F 1B F0 1B
	/* t T           07 */ 0x17, 0x14, // 94 2C F0 2C
	/* u U           07 */ 0x18, 0x16, // 96 3C F0 3C
	/* v V           07 */ 0x19, 0x2F, // AF 2A F0 2A
	/* w W           07 */ 0x1A, 0x11, // 91 1D F0 1D
	/* x X           07 */ 0x1B, 0x2D, // AD 22 F0 22
	/* y Y           07 */ 0x1C, 0x15, // 95 35 F0 35
	/* z Z           07 */ 0x1D, 0x2C, // AC 1A F0 1A
	/* 1 !           07 */ 0x1E, 0x02, // 82 16 F0 16
	/* 2 @           07 */ 0x1F, 0x03, // 83 1E F0 1E
	/* 3 #           07 */ 0x20, 0x04, // 84 26 F0 26
	/* 4 $           07 */ 0x21, 0x05, // 85 25 F0 25
	/* 5 %           07 */ 0x22, 0x06, // 86 2E F0 2E
	/* 6 ^           07 */ 0x23, 0x07, // 87 36 F0 36
	/* 7 &           07 */ 0x24, 0x08, // 88 3D F0 3D
	/* 8 *           07 */ 0x25, 0x09, // 89 3E F0 3E
	/* 9 (           07 */ 0x26, 0x0A, // 8A 46 F0 46
	/* 0 )           07 */ 0x27, 0x0B, // 8B 45 F0 45
	/* Return        07 */ 0x28, 0x1C, // 9C 5A F0 5A
	/* Escape        07 */ 0x29, 0x01, // 81 76 F0 76
	/* Backspace     07 */ 0x2A, 0x0E, // 8E 66 F0 66
	/* Tab           07 */ 0x2B, 0x0F, // 8F 0D F0 0D
	/* Space         07 */ 0x2C, 0x39, // B9 29 F0 29
	/* - _           07 */ 0x2D, 0x0C, // 8C 4E F0 4E
	/* = +           07 */ 0x2E, 0x0D, // 8D 55 F0 55
	/* [ {           07 */ 0x2F, 0x1A, // 9A 54 F0 54
	/* ] }           07 */ 0x30, 0x1B, // 9B 5B F0 5B
	/* \ |           07 */ 0x31, 0x2B, // AB 5D F0 5D
	/* Europe1 Note2 07 */ 0x32, 0x2B, // AB 5D F0 5D
	/* ; :           07 */ 0x33, 0x27, // A7 4C F0 4C
	/* ' "           07 */ 0x34, 0x28, // A8 52 F0 52
	/* ` ~           07 */ 0x35, 0x29, // A9 0E F0 0E
	/* , <           07 */ 0x36, 0x33, // B3 41 F0 41
	/* . >           07 */ 0x37, 0x34, // B4 49 F0 49
	/* / ?           07 */ 0x38, 0x35, // B5 4A F0 4A
	/* Caps Lock     07 */ 0x39, 0x3A, // BA 58 F0 58
	/* F1            07 */ 0x3A, 0x3B, // BB 05 F0 05
	/* F2            07 */ 0x3B, 0x3C, // BC 06 F0 06
	/* F3            07 */ 0x3C, 0x3D, // BD 04 F0 04
	/* F4            07 */ 0x3D, 0x3E, // BE 0C F0 0C
	/* F5            07 */ 0x3E, 0x3F, // BF 03 F0 03
	/* F6            07 */ 0x3F, 0x40, // C0 0B F0 0B
	/* F7            07 */ 0x40, 0x41, // C1 83 F0 83
	/* F8            07 */ 0x41, 0x42, // C2 0A F0 0A
	/* F9            07 */ 0x42, 0x43, // C3 01 F0 01
	/* F10           07 */ 0x43, 0x44, // C4 09 F0 09
	/* F11           07 */ 0x44, 0x57, // D7 78 F0 78
	/* F12           07 */ 0x45, 0x58, // D8 07 F0 07
	/* PrScr Note1   07 */ 0x46, 0xE037, // E0 B7 E0 7C E0 F0 7C
	/* Scroll Lock   07 */ 0x47, 0x46, // C6 7E F0 7E
	/* Brk CtrlPause 07 */ 0x48, 0xE046, // E0 C6 None E0 7E E0 F0 7E None
	/* Pause         07 */ 0x48, 0xE11D, // 45 E1 9D C5 None E1 14 77 E1 F0 14 F0 77 None 
	/* Insert Note1  07 */ 0x49, 0xE052, // E0 D2 E0 70 E0 F0 70
	/* Home Note1    07 */ 0x4A, 0xE047, // E0 C7 E0 6C E0 F0 6C
	/* Page Up Note1 07 */ 0x4B, 0xE049, // E0 C9 E0 7D E0 F0 7D
	/* Delete Note1  07 */ 0x4C, 0xE053, // E0 D3 E0 71 E0 F0 71
	/* End (Note 1)  07 */ 0x4D, 0xE04F, // E0 CF E0 69 E0 F0 69
	/* PgDown Note1  07 */ 0x4E, 0xE051, // E0 D1 E0 7A E0 F0 7A
	/* RightArrNote1 07 */ 0x4F, 0xE04D, // E0 CD E0 74 E0 F0 74
	/* LeftArr Note1 07 */ 0x50, 0xE04B, // E0 CB E0 6B E0 F0 6B
	/* DownArr Note1 07 */ 0x51, 0xE050, // E0 D0 E0 72 E0 F0 72
	/* UpArrow Note1 07 */ 0x52, 0xE048, // E0 C8 E0 75 E0 F0 75
	/* Num Lock      07 */ 0x53, 0x45,   // C5 77 F0 77
	/* Keypad/ Note1 07 */ 0x54, 0xE035, // E0 B5 E0 4A E0 F0 4A
	/* Keypad *      07 */ 0x55, 0x37,   // B7 7C F0 7C
	/* Keypad -      07 */ 0x56, 0x4A,   // CA 7B F0 7B
	/* Keypad +      07 */ 0x57, 0x4E,   // CE 79 F0 79
	/* Keypad Enter  07 */ 0x58, 0xE01C, // E0 9C E0 5A E0 F0 5A
	/* Keypad 1 End  07 */ 0x59, 0x4F,   // CF 69 F0 69
	/* Keypad 2 Down 07 */ 0x5A, 0x50,   // D0 72 F0 72
	/* Keypad 3 PgDn 07 */ 0x5B, 0x51,   // D1 7A F0 7A
	/* Keypad 4 Left 07 */ 0x5C, 0x4B,   // CB 6B F0 6B
	/* Keypad 5      07 */ 0x5D, 0x4C,   // CC 73 F0 73
	/* Keypad 6 Rght 07 */ 0x5E, 0x4D,   // CD 74 F0 74
	/* Keypad 7 Home 07 */ 0x5F, 0x47,   // C7 6C F0 6C
	/* Keypad 8 Up   07 */ 0x60, 0x48,   // C8 75 F0 75
	/* Keypad 9 PgUp 07 */ 0x61, 0x49,   // C9 7D F0 7D
	/* Keypad 0 Ins  07 */ 0x62, 0x52,   // D2 70 F0 70
	/* Keypad . Del  07 */ 0x63, 0x53,   // D3 71 F0 71
	/* Europe2 Note2 07 */ 0x64, 0x56,   // D6 61 F0 61
	/* App           07 */ 0x65, 0xE05D, // E0 DD E0 2F E0 F0 2F
	/* KeyboardPower 07 */ 0x66, 0xE05E, // E0 DE E0 37 E0 F0 37
	/* Keypad =      07 */ 0x67, 0x59,   // D9 0F F0 0F
	/* F13           07 */ 0x68, 0x64,   // E4 08 F0 08
	/* F14           07 */ 0x69, 0x65,   // E5 10 F0 10
	/* F15           07 */ 0x6A, 0x66,   // E6 18 F0 18
	/* F16           07 */ 0x6B, 0x67,   // E7 20 F0 20
	/* F17           07 */ 0x6C, 0x68,   // E8 28 F0 28
	/* F18           07 */ 0x6D, 0x69,   // E9 30 F0 30
	/* F19           07 */ 0x6E, 0x6A,   // EA 38 F0 38
	/* F20           07 */ 0x6F, 0x6B,   // EB 40 F0 40
	/* F21           07 */ 0x70, 0x6C,   // EC 48 F0 48
	/* F22           07 */ 0x71, 0x6D,   // ED 50 F0 50
	/* F23           07 */ 0x72, 0x6E,   // EE 57 F0 57
	/* F24           07 */ 0x73, 0x76,   // F6 5F F0 5F
	/* Left Control  07 */ 0xE0, 0x1D,   // 9D 14 F0 14
	/* Left Shift    07 */ 0xE1, 0x2A,   // AA 12 F0 12
	/* Left Alt      07 */ 0xE2, 0x38,   // B8 11 F0 11
	/* Left GUI      07 */ 0xE3, 0xE05B, // E0 DB E0 1F E0 F0 1F
	/* Right Control 07 */ 0xE4, 0xE01D, // E0 9D E0 14 E0 F0 14
	/* Right Shift   07 */ 0xE5, 0x36,   // B6 59 F0 59
	/* Right Alt     07 */ 0xE6, 0xE038, // E0 B8 E0 11 E0 F0 11
	/* Right GUI     07 */ 0xE7, 0xE05C  // E0 DC E0 27 E0 F0 27
};

void CUsbDevice::setHost( int idx, CUsbHost* ptr)
{
	usbHost = ptr;
	index = idx;
}

CUsbDevice::CUsbDevice()
{
}

CUsbDevice::~CUsbDevice()
{
}

void CUsbDevice::processKeybPacket(unsigned char* pkt)
{
	/*
	for (int i = 0; i < 8; i++)
	{
		printf("%02X ", pkt[i]);
	}
	*/
	printf("\n");
	memcpy(prev_keyb_packet, curr_keyb_packet, HID_NUM_CODES);
	memcpy(curr_keyb_packet, pkt, 8);
	//unroll modifiers
	memset(&curr_keyb_packet[8], 0, 8);
	if (pkt[0] & Modifier_Left_Ctrl)  curr_keyb_packet[8] = 0xE0;
	if (pkt[0] & Modifier_Left_Shift)  curr_keyb_packet[9] = 0xE1;
	if (pkt[0] & Modifier_Left_Alt)  curr_keyb_packet[10] = 0xE2;
	if (pkt[0] & Modifier_Left_GUI)  curr_keyb_packet[11] = 0xE3;
	if (pkt[0] & Modifier_Right_Ctrl)  curr_keyb_packet[12] = 0xE4;
	if (pkt[0] & Modifier_Right_Shift) curr_keyb_packet[13] = 0xE5;
	if (pkt[0] & Modifier_Right_Alt)  curr_keyb_packet[14] = 0xE6;
	if (pkt[0] & Modifier_Right_GUI)  curr_keyb_packet[15] = 0xE7;
}

void CUsbDevice::processKeybUp()
{
	uint8_t* prev_pkt = prev_keyb_packet;
	uint8_t* pkt = curr_keyb_packet;
	//find codes which released
	//these codes were in prev table, but dissapear in new table
	for (int k = 2; k < HID_NUM_CODES; k++)
	{
		uint8_t hid_code = prev_pkt[k];
		bool found = false;
		for (int q = 2; q < HID_NUM_CODES; q++)
		{
			if (hid_code == pkt[q])
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			//make key release
			int N = sizeof(hid2scan) / sizeof(uint32_t) / 2;
			for (int i = 0; i < N; i++)
			{
				if (hid2scan[i * 2] == hid_code)
				{
					auto it = key_press_map.find(hid_code);
					if (it != key_press_map.end())
						key_press_map.erase(it);
					BYTE scan = hid2scan[i * 2 + 1] & 0xFF;
					BYTE Ext = hid2scan[i * 2 + 1] >> 8;
					BYTE vk = MapVirtualKeyA(scan, MAPVK_VSC_TO_VK_EX);
					DWORD flags = KEYEVENTF_KEYUP | (Ext ? KEYEVENTF_EXTENDEDKEY : 0);
					ULONG_PTR ptr = 0;
					keybd_event(vk, scan, flags, ptr);
					printf("up %02X\n", hid_code);
					break;
				}
			}
		}
	}
}

void CUsbDevice::processKeybDown()
{
	uint8_t* pkt = curr_keyb_packet;
	for (int k = 2; k < HID_NUM_CODES; k++)
	{
		uint8_t hid_code = pkt[k];
		if (hid_code)
		{
			//make key press
			int N = sizeof(hid2scan) / sizeof(uint32_t) / 2;
			for (int i = 0; i < N; i++)
			{
				if (hid2scan[i * 2] == hid_code)
				{
					auto it = key_press_map.find(hid_code);
					bool do_press = false;
					if (it == key_press_map.end())
					{
						//this is new code, not pressed yet, remember time of first press
						do_press = true;
						key_press_map[hid_code] = chrono::steady_clock::now();
					}
					else
						if (hid_code<0xE0) //no repeate for modifiers
						{
							//can repeat press if elapsed enough guard time
							auto t0 = it->second;
							auto t1 = chrono::steady_clock::now();
							auto elapsed = chrono::duration_cast<std::chrono::milliseconds> (t1 - t0).count();
							if (elapsed > 200)
								do_press = true;
						}

					if (do_press)
					{
						BYTE scan = hid2scan[i * 2 + 1] & 0xFF;
						BYTE Ext = hid2scan[i * 2 + 1] >> 8;
						BYTE vk = MapVirtualKeyA(scan, MAPVK_VSC_TO_VK_EX);
						DWORD flags = Ext ? KEYEVENTF_EXTENDEDKEY : 0;
						ULONG_PTR ptr = 0;
						keybd_event(vk, scan, flags, ptr);
						printf("dn %02X\n", hid_code);
					}
					break;
				}
			}
		}
	}
}

bool CUsbDevice::readPipeKeyb()
{
	unsigned char sbuffer[] = {
		CMD_WAIT_EOF,
		CMD_WAIT_EOF,
		0x40 | CMD_SEND_PKT, 0x80, 0x69, 0x81, 0x58,
		CMD_READ_PKT,
		CMD_AUTO_ACK
	};
	usbHost->send(sizeof(sbuffer), sbuffer);
	unsigned char rbuffer[32];
	int got_length = 0;
	bool r = usbHost->recvPacket(rbuffer, sizeof(rbuffer), &got_length, 100);
	if (!r) return false;
	if (got_length == 2 && rbuffer[1] == 0x5A)
	{
		//no data NAK
	}
	else
	{
		processKeybPacket(&rbuffer[2]);
		processKeybUp();
	}
	processKeybDown();
	return true;
}

void CUsbDevice::processMousePacket(unsigned char* pkt)
{
	short dx, dy;
	dx = pkt[1] | (pkt[2] << 8);
	dy = pkt[3] | (pkt[4] << 8);

	unsigned int flag = MOUSEEVENTF_MOVE;

	if (pkt[0] & 1)
	{
		if (!ldown)
		{
			flag = flag | MOUSEEVENTF_LEFTDOWN;
			ldown = true;
		}
	}
	else
	{
		if (ldown)
		{
			flag = flag | MOUSEEVENTF_LEFTUP;
			ldown = false;
		}
	}

	if (pkt[0] & 2)
	{
		if (!rdown)
		{
			flag = flag | MOUSEEVENTF_RIGHTDOWN;
			rdown = true;
		}
	}
	else
	{
		if (rdown)
		{
			flag = flag | MOUSEEVENTF_RIGHTUP;
			rdown = false;
		}
	}

	mouse_event(flag, dx, dy, 0, 0);
}

bool CUsbDevice::readPipeMouse()
{
	unsigned char sbuffer[] = {
		CMD_WAIT_EOF,
		CMD_WAIT_EOF,
		0x40 | CMD_SEND_PKT, 0x80, 0x69, 0x81, 0x58,
		CMD_READ_PKT,
		CMD_AUTO_ACK
	};
	usbHost->send(sizeof(sbuffer), sbuffer);
	unsigned char rbuffer[32];
	int got_length = 0;
	bool r = usbHost->recvPacket(rbuffer, sizeof(rbuffer), &got_length, 100);
	if (!r) return false;
	if (got_length == 2 && rbuffer[1] == 0x5A)
		return true; //no data NAK
	processMousePacket(&rbuffer[2]);
	return true;
}

bool CUsbDevice::poll()
{
	unsigned char cmd;
	unsigned char cmds[16];
	DWORD got;
	bool r;
	unsigned char lines_status;
	switch (state)
	{
		case(HOST_STATE_IDLE):
		{
			cmd = CMD_GET_LINES;
			usbHost->send(1, &cmd);

			lines_status = 0;
			r = usbHost->readByte(&lines_status, 100);
			if (r)
			{
				//controller replied with lines status
				//printf("%02X\n", lines_status);
				if (((lines_status & 0x0F) == CMD_GET_LINES) && ((lines_status & 0xF0) != 0x00))
				{
					printf("Attached!\n");
					state = HOST_STATE_RESET;
				}
			}
			break;
		}
		case(HOST_STATE_RESET):
		{
			//USB reset, then wait nearest SE0/EOF, then read USB lines status
			printf("USB Reset\n");
			cmds[0] = 0x30 | CMD_BUS_CTRL;
			cmds[1] = CMD_WAIT_EOF;
			cmds[2] = CMD_GET_LINES;
			usbHost->send(3, &cmds[0]);

			lines_status = 0;
			r = usbHost->readByte(&lines_status, 100);
			if (r)
			{
				printf("%02X\n", lines_status);
			}
			else
			{
				printf("Read error\n");
				state = HOST_STATE_IDLE;
			}
			state = HOST_STATE_RESET1;
			break;
		}
		case(HOST_STATE_RESET1):
		{
			state = HOST_STATE_RESET2;
			break;
		}
		case(HOST_STATE_RESET2):
		{
			state = HOST_STATE_ENABLE;
			break;
		}
		case(HOST_STATE_ENABLE):
		{
			//USB enable, then wait nearest SE0/EOF, then read USB lines status
			printf("USB Reset\n");
			cmds[0] = 0x20 | CMD_BUS_CTRL;
			cmds[1] = CMD_WAIT_EOF;
			cmds[2] = CMD_GET_LINES;
			usbHost->send(3, &cmds[0]);

			lines_status = 0;
			r = usbHost->readByte(&lines_status, 100);
			if (r)
			{
				printf("%02X\n", lines_status);
			}
			else
			{
				printf("Read error\n");
				state = HOST_STATE_IDLE;
			}
			state = HOST_STATE_GET_CONFIG;
			break;
		}
		case(HOST_STATE_GET_CONFIG):
		{
			unsigned char descriptor[256];
			int got_length = 0;
			usbHost->getDescriptor0(descriptor, sizeof(descriptor), &got_length);
			state = HOST_STATE_SET_ADDRESS;
			break;
		}
		case(HOST_STATE_SET_ADDRESS):
		{
			usbHost->setAddress();
			state = HOST_STATE_SET_CONFIGURATION;
			break;
		}
		case(HOST_STATE_SET_CONFIGURATION):
		{
			usbHost->setConfiguration();
			state = HOST_STATE_POLL;
			break;
		}
		case(HOST_STATE_POLL):
		{
			if (index == 0)
			{
				if (!readPipeKeyb())
					state = HOST_STATE_IDLE;
			}
			else
			{
				if (!readPipeMouse())
					state = HOST_STATE_IDLE;
			}
			break;
		}
	}
	return true;
}
