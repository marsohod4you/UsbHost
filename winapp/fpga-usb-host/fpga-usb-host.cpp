// fpga-usb-host.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

//protocol definition
#define CMD_GET_LINES 1
#define CMD_BUS_CTRL 2
#define CMD_WAIT_EOF 3
#define CMD_SEND_PKT 4
#define CMD_READ_PKT 5
#define CMD_AUTO_ACK 6

HANDLE hThread;
HANDLE hPort = NULL;
DWORD dwError;

int Init(LPCWSTR lpszPortName, int baud)
{
	//DWORD dwThreadID;
	DCB PortDCB;
	COMMTIMEOUTS CommTimeouts;

	dwError = 0;

	// Open the serial port.
	hPort = CreateFile(lpszPortName, // Pointer to the name of the port
		GENERIC_READ | GENERIC_WRITE,
		// Access (read/write) mode
		0,            // Share mode
		NULL,         // Pointer to the security attribute
		OPEN_EXISTING,// How to open the serial port
		0, //FILE_FLAG_OVERLAPPED, // Port attributes
		NULL);        // Handle to port with attribute
					  // to copy

					  // If it fails to open the port, return FALSE.
	if (hPort == INVALID_HANDLE_VALUE)
	{
		// Could not open the port.
		dwError = GetLastError();
		//MessageBox (NULL, TEXT("Unable to open the port specified..."),TEXT("Error"), MB_OK);
		return dwError;
	}

	PortDCB.DCBlength = sizeof(DCB);

	// Get the default port setting information.
	GetCommState(hPort, &PortDCB);

	// Change the DCB structure settings.
	PortDCB.BaudRate = baud;         // Current baud 
	PortDCB.fBinary = TRUE;               // Binary mode; no EOF check 
	PortDCB.fParity = TRUE;               // Enable parity checking. 
	PortDCB.fOutxCtsFlow = FALSE;         // No CTS output flow control 
	PortDCB.fOutxDsrFlow = FALSE;         // No DSR output flow control 
	PortDCB.fDtrControl = DTR_CONTROL_DISABLE;
	// DTR flow control type 
	PortDCB.fDsrSensitivity = FALSE;      // DSR sensitivity 
	PortDCB.fTXContinueOnXoff = TRUE;     // XOFF continues Tx 
	PortDCB.fOutX = FALSE;                // No XON/XOFF out flow control 
	PortDCB.fInX = FALSE;                 // No XON/XOFF in flow control 
	PortDCB.fErrorChar = FALSE;           // Disable error replacement. 
	PortDCB.fNull = FALSE;                // Disable null stripping. 
	PortDCB.fRtsControl = RTS_CONTROL_DISABLE;
	// RTS flow control 
	PortDCB.fAbortOnError = FALSE;        // Do not abort reads/writes on 
										  // error.
	PortDCB.ByteSize = 8;                 // Number of bits/bytes, 4-8 
	PortDCB.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space 
	PortDCB.StopBits = TWOSTOPBITS;        // 0,1,2 = 1, 1.5, 2 

										  // Configure the port according to the specifications of the DCB 
										  // structure.
	if (!SetCommState(hPort, &PortDCB))
	{
		// Could not configure the serial port.

		dwError = GetLastError();
		//MessageBox (NULL, TEXT("Unable to configure the serial port"), TEXT("Error"), MB_OK);
		return dwError;
	}

	// Retrieve the time-out parameters for all read and write operations
	// on the port. 
	GetCommTimeouts(hPort, &CommTimeouts);

	// Change the COMMTIMEOUTS structure settings.
	CommTimeouts.ReadIntervalTimeout = MAXDWORD;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.WriteTotalTimeoutMultiplier = 10;
	CommTimeouts.WriteTotalTimeoutConstant = 1000;

	// Set the time-out parameters for all read and write operations
	// on the port. 
	if (!SetCommTimeouts(hPort, &CommTimeouts))
	{
		// Could not set the time-out parameters.

		dwError = GetLastError();
		//MessageBox (NULL, TEXT("Unable to set the time-out parameters"), TEXT("Error"), MB_OK);
		return dwError;
	}

	// Direct the port to perform extended functions SETDTR and SETRTS.
	// SETDTR: Sends the DTR (data-terminal-ready) signal.
	// SETRTS: Sends the RTS (request-to-send) signal. 
	EscapeCommFunction(hPort, SETDTR);
	EscapeCommFunction(hPort, SETRTS);
	return 0;
}

void send(int len, unsigned char* buffer)
{
	DWORD wr = 0;
	WriteFile(hPort, buffer, len, &wr, NULL);
}

//try to read serial port one byte for some time (in ms)
bool read_byte(unsigned char* byte, int timeout )
{
	unsigned char rbuffer;
	DWORD got = 0;
	int n = 0;
	int sleep_ms = 10;
	while (got == 0)
	{
		ReadFile(hPort, &rbuffer, 1, &got, NULL);
		if (got == 1)
		{
			byte[0] = rbuffer;
			return true;
		}
		if ( n*sleep_ms > timeout )
			return false;;
		n++;
		Sleep(sleep_ms);
	}
}

bool recv_packet( unsigned char* rbuffer, int rbuffer_length, int* got_length, int timeout )
{
	//first should CMD_READ_PKT command come
	unsigned char cmd;
	bool r = read_byte(&cmd, timeout);
	if (r)
	{
		printf("%02X\n", cmd);
	}
	else
	{
		printf("Read error\n");
		return false;
	}
	if ((cmd & 0x0F) != CMD_READ_PKT)
	{
		printf("No expected CMD_READ_PKT cmd\n");
		return false;
	}
	int pkt_length = (cmd & 0xF0) >> 4;
	if (pkt_length == 0 || pkt_length>rbuffer_length)
	{
		printf("Wrong recv pkt_length=%d\n", pkt_length);
		return false;
	}

	printf("Received packet: ");
	for (int i = 0; i < pkt_length; i++)
	{
		rbuffer[i] = 0;
		bool r = read_byte(&rbuffer[i], timeout);
		if (!r)
		{
			printf("Cannot read pkt byte\n");
			return false;
		}
		printf("%02X ", rbuffer[i]);
	}
	*got_length = pkt_length;
	printf("\n");
	return true;
}

bool get_descriptor0(unsigned char* descriptor, int buf_length, int* actual_length )
{
	unsigned char sbuffer[] = { 
		CMD_WAIT_EOF,
		CMD_WAIT_EOF,
		0x40 | CMD_SEND_PKT, 0x80, 0x2d, 0x00, 0x10,
		0xC0 | CMD_SEND_PKT, 0x80, 0xc3, 0x80, 0x06, 0x00, 0x01, 0x00, 0x00, 0x40, 0x00, 0xdd, 0x94,
		CMD_READ_PKT
	};
	send(sizeof(sbuffer), sbuffer);
	unsigned char rbuffer[32];
	int got_length = 0;
	bool r = recv_packet(rbuffer, sizeof(rbuffer), &got_length, 100);
	if (!r) return false;
	//TODO: check that ACK received here
	if (rbuffer[0] == 0x80 && rbuffer[1] == 0xD2)
	{
		//got ACK, so fine
	}

	int idx = 0;
	for (int i = 0; i < 8; i++)
	{
		unsigned char sbuffer_in[] = {
			CMD_WAIT_EOF,
			0x40 | CMD_SEND_PKT, 0x80, 0x69, 0x00, 0x10,
			CMD_READ_PKT,
			CMD_AUTO_ACK
		};
		send(sizeof(sbuffer_in), sbuffer_in);
		got_length = 0;
		r = recv_packet(rbuffer, sizeof(rbuffer), &got_length, 100);
		if (!r) return false;
		//short packet detect
		if (idx + got_length > buf_length)
		{
			//ops, error, caller gives not enough memory..
			return false;
		}
		memcpy(&descriptor[idx], rbuffer, got_length);
		idx += got_length;
		if (got_length < 12)
			break;
	}
	*actual_length = idx;
	//complete get_descriptor
	unsigned char sbuffer_out[] = {
		CMD_WAIT_EOF,
		CMD_WAIT_EOF,
		0x40 | CMD_SEND_PKT, 0x80, 0xE1, 0x00, 0x10,
		0x40 | CMD_SEND_PKT, 0x80, 0x4B, 0x00, 0x00,
		CMD_READ_PKT
	};
	send(sizeof(sbuffer_out), sbuffer_out);
	got_length = 0;
	r = recv_packet(rbuffer, sizeof(rbuffer), &got_length, 100);
	if (!r) return false;
	//TODO: check that ACK received here
	if (rbuffer[0] == 0x80 && rbuffer[1] == 0xD2)
	{
		//got ACK, so fine
	}
	return true;
}

bool set_address()
{
	unsigned char sbuffer[] = {
		CMD_WAIT_EOF,
		CMD_WAIT_EOF,
		0x40 | CMD_SEND_PKT, 0x80, 0x2d,0x00, 0x10,
		0xC0 | CMD_SEND_PKT, 0x80, 0xc3, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEB, 0x25,
		CMD_READ_PKT
	};
	send(sizeof(sbuffer), sbuffer);
	unsigned char rbuffer[32];
	int got_length = 0;
	bool r = recv_packet(rbuffer, sizeof(rbuffer), &got_length, 100);
	if (!r) return false;
	//TODO: check that ACK received here
	if (rbuffer[0] == 0x80 && rbuffer[1] == 0xD2)
	{
		//got ACK, so fine
	}

	unsigned char sbuffer_in[] = {
		CMD_WAIT_EOF,
		0x40 | CMD_SEND_PKT, 0x80, 0x69, 0x00, 0x10,
		CMD_READ_PKT,
		CMD_AUTO_ACK
	};
	send(sizeof(sbuffer_in), sbuffer_in);
	got_length = 0;
	r = recv_packet(rbuffer, sizeof(rbuffer), &got_length, 100);
	if (!r) return false;
	
	return true;
}

bool set_configuration()
{
	unsigned char sbuffer[] = {
		CMD_WAIT_EOF,
		CMD_WAIT_EOF,
		0x40 | CMD_SEND_PKT, 0x80, 0x2d, 0x01, 0xE8,
		0xC0 | CMD_SEND_PKT, 0x80, 0xc3, 0x00, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x25,
		CMD_READ_PKT
	};
	send(sizeof(sbuffer), sbuffer);
	unsigned char rbuffer[32];
	int got_length = 0;
	bool r = recv_packet(rbuffer, sizeof(rbuffer), &got_length, 100);
	if (!r) return false;
	//TODO: check that ACK received here
	if (rbuffer[0] == 0x80 && rbuffer[1] == 0xD2)
	{
		//got ACK, so fine
	}

	unsigned char sbuffer_in[] = {
		CMD_WAIT_EOF,
		0x40 | CMD_SEND_PKT, 0x80, 0x69, 0x01, 0xE8,
		CMD_READ_PKT,
		CMD_AUTO_ACK
	};
	send(sizeof(sbuffer_in), sbuffer_in);
	got_length = 0;
	r = recv_packet(rbuffer, sizeof(rbuffer), &got_length, 100);
	if (!r) return false;

	return true;
}


bool ldown = false;
bool rdown = false;
void process_mouse_packet( unsigned char* pkt)
{
	short dx, dy;
	dx = pkt[1] | (pkt[2]<<8);
	dy = pkt[3] | (pkt[4]<<8);
	
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

bool read_pipe()
{
	unsigned char sbuffer[] = {
		CMD_WAIT_EOF,
		CMD_WAIT_EOF,
		0x40 | CMD_SEND_PKT, 0x80, 0x69, 0x81, 0x58,
		CMD_READ_PKT,
		CMD_AUTO_ACK
	};
	send(sizeof(sbuffer), sbuffer);
	unsigned char rbuffer[32];
	int got_length = 0;
	bool r = recv_packet(rbuffer, sizeof(rbuffer), &got_length, 100);
	if (!r) return false;
	if (got_length == 2 && rbuffer[1] == 0x5A)
		return true; //no data NAK
	process_mouse_packet( &rbuffer[2] );
	return true;
}

int main()
{
	Init(L"\\\\.\\COM18",921600);
	PurgeComm(hPort, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
	unsigned char cmd;
	unsigned char cmds[16];
	DWORD got;
	bool r;
	unsigned char lines_status;

	//wait device attach
	while (1)
	{
		cmd = CMD_GET_LINES;
		send(1, &cmd);

		lines_status=0;
		r = read_byte(&lines_status, 100);
		if (r)
		{
			//controller replied with lines status
			printf("%02X\n", lines_status);
			if ( ((lines_status &0x0F) == CMD_GET_LINES) && ((lines_status &0xF0) != 0x00) )
			{
				printf("Attached!\n");
				break;
			}
			Sleep(1000);
		}

	}

	//USB reset, then wait nearest SE0/EOF, then read USB lines status
	printf("USB Reset\n");
	cmds[0] = 0x30 | CMD_BUS_CTRL;
	cmds[1] = CMD_WAIT_EOF;
	cmds[2] = CMD_GET_LINES;
	send(3, &cmds[0]);

	lines_status = 0;
	r = read_byte(&lines_status, 100);
	if (r)
	{
		printf("%02X\n", lines_status);
	}
	else
	{
		printf("Read error\n");
	}

	Sleep(20);

	//USB enable, then wait nearest SE0/EOF, then read USB lines status
	printf("USB Reset\n");
	cmds[0] = 0x20 | CMD_BUS_CTRL;
	cmds[1] = CMD_WAIT_EOF;
	cmds[2] = CMD_GET_LINES;
	send(3, &cmds[0]);

	lines_status = 0;
	r = read_byte(&lines_status, 100);
	if (r)
	{
		printf("%02X\n", lines_status);
	}
	else
	{
		printf("Read error\n");
	}

	unsigned char descriptor[256];
	int got_length = 0;
	get_descriptor0(descriptor,sizeof(descriptor),&got_length);
	set_address();
	set_configuration();
	while (1)
	{
		read_pipe();
		Sleep(20);
	}
	return 0;
}
