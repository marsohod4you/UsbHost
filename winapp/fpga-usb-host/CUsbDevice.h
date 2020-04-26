#pragma once
#include <windows.h>
#include <stdint.h>
#include <map>
#include <chrono>

using namespace std;

//protocol definition
#define CMD_GET_LINES 1
#define CMD_BUS_CTRL 2
#define CMD_WAIT_EOF 3
#define CMD_SEND_PKT 4
#define CMD_READ_PKT 5
#define CMD_AUTO_ACK 6

//HID keyboard modifier flags
#define	Modifier_Left_Ctrl	 0x01
#define	Modifier_Left_Shift  0x02
#define	Modifier_Left_Alt	 0x04
#define	Modifier_Left_GUI	 0x08
#define	Modifier_Right_Ctrl	 0x10
#define	Modifier_Right_Shift 0x20
#define	Modifier_Right_Alt   0x40
#define	Modifier_Right_GUI   0x80

#define HID_NUM_CODES 16

enum HOST_STATE {
	HOST_STATE_IDLE = 0,
	HOST_STATE_RESET,
	HOST_STATE_RESET1,
	HOST_STATE_RESET2,
	HOST_STATE_ENABLE,
	HOST_STATE_GET_CONFIG,
	HOST_STATE_SET_ADDRESS,
	HOST_STATE_SET_CONFIGURATION,
	HOST_STATE_POLL
};

class CUsbHost;

class CUsbDevice
{
public:
	CUsbDevice();
	~CUsbDevice();
	void setHost(int idx, CUsbHost* ptr);
	bool poll();
private:
	bool readPipeKeyb();
	bool readPipeMouse();
	void processKeybPacket(unsigned char* pkt);
	void processKeybUp();
	void processKeybDown();
	void processMousePacket(unsigned char* pkt);

	CUsbHost* usbHost{nullptr};
	HOST_STATE state{ HOST_STATE_IDLE };
	int index{ 0 };

	//usb keyboard specific fields
	uint8_t prev_keyb_packet[16] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
	uint8_t curr_keyb_packet[16] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
	map< uint8_t, chrono::steady_clock::time_point> key_press_map;
	//usb mouse specific fields
	bool ldown = false;
	bool rdown = false;
};

