#pragma once
#include <windows.h>
#include <thread>

using namespace std;

class CUsbDevice;

class CUsbHost
{
public:
	CUsbHost();
	~CUsbHost();
	int init(LPCWSTR lpszPortName, int baud);
	void send(int len, unsigned char* buffer);
	bool readByte(unsigned char* byte, int timeout);
	bool recvPacket(unsigned char* rbuffer, int rbuffer_length, int* got_length, int timeout);
	bool getDescriptor0(unsigned char* descriptor, int buf_length, int* actual_length);
	bool setAddress();
	bool setConfiguration();
private:
	bool run;
	thread hostThread;
	HANDLE hPort = NULL;
	DWORD dwError;
	CUsbDevice* usbDevice[2];
	friend void usbHostThread(void* ptr);
};

