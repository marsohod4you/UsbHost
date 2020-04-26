#include "stdafx.h"
#include "CUsbHost.h"
#include "CUsbDevice.h"

CUsbHost::CUsbHost()
{
}

CUsbHost::~CUsbHost()
{
	run = false;
	hostThread.join();
	if (usbDevice[0])
		delete usbDevice[0];
	if (usbDevice[1])
		delete usbDevice[1];
}

void usbHostThread( void* ptr )
{
	CUsbHost* host = (CUsbHost*)ptr;
	while (host->run)
	{
		this_thread::sleep_for(10ms);
		EscapeCommFunction(host->hPort, SETRTS);
		host->usbDevice[0]->poll();

		this_thread::sleep_for(10ms);
		EscapeCommFunction(host->hPort, CLRRTS);
		host->usbDevice[1]->poll();
	}
}

int CUsbHost::init(LPCWSTR lpszPortName, int baud)
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

	usbDevice[0] = new CUsbDevice;
	usbDevice[1] = new CUsbDevice;
	usbDevice[0]->setHost(0,this);
	usbDevice[1]->setHost(1,this);
	run = true;
	hostThread = thread( usbHostThread, this );
	return 0;
}

void CUsbHost::send(int len, unsigned char* buffer)
{
	DWORD wr = 0;
	WriteFile(hPort, buffer, len, &wr, NULL);
}

//try to read serial port one byte for some time (in ms)
bool CUsbHost::readByte(unsigned char* byte, int timeout)
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
		if (n*sleep_ms > timeout)
			return false;;
		n++;
		Sleep(sleep_ms);
	}
	return true;
}

bool CUsbHost::recvPacket(unsigned char* rbuffer, int rbuffer_length, int* got_length, int timeout)
{
	//first should CMD_READ_PKT command come
	unsigned char cmd;
	bool r = readByte(&cmd, timeout);
	if (r)
	{
		//printf("%02X\n", cmd);
	}
	else
	{
		//printf("Read error\n");
		return false;
	}
	if ((cmd & 0x0F) != CMD_READ_PKT)
	{
		//printf("No expected CMD_READ_PKT cmd\n");
		return false;
	}
	int pkt_length = (cmd & 0xF0) >> 4;
	if (pkt_length == 0 || pkt_length>rbuffer_length)
	{
		//printf("Wrong recv pkt_length=%d\n", pkt_length);
		return false;
	}

	//printf("Received packet: ");
	for (int i = 0; i < pkt_length; i++)
	{
		rbuffer[i] = 0;
		bool r = readByte(&rbuffer[i], timeout);
		if (!r)
		{
			//printf("Cannot read pkt byte\n");
			return false;
		}
		//printf("%02X ", rbuffer[i]);
	}
	*got_length = pkt_length;
	//printf("\n");
	return true;
}

bool CUsbHost::getDescriptor0(unsigned char* descriptor, int buf_length, int* actual_length)
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
	bool r = recvPacket(rbuffer, sizeof(rbuffer), &got_length, 100);
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
		r = recvPacket(rbuffer, sizeof(rbuffer), &got_length, 100);
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
	r = recvPacket(rbuffer, sizeof(rbuffer), &got_length, 100);
	if (!r) return false;
	//TODO: check that ACK received here
	if (rbuffer[0] == 0x80 && rbuffer[1] == 0xD2)
	{
		//got ACK, so fine
	}
	return true;
}

bool CUsbHost::setAddress()
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
	bool r = recvPacket(rbuffer, sizeof(rbuffer), &got_length, 100);
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
	r = recvPacket(rbuffer, sizeof(rbuffer), &got_length, 100);
	if (!r) return false;

	return true;
}

bool CUsbHost::setConfiguration()
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
	bool r = recvPacket(rbuffer, sizeof(rbuffer), &got_length, 100);
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
	r = recvPacket(rbuffer, sizeof(rbuffer), &got_length, 100);
	if (!r) return false;

	return true;
}
