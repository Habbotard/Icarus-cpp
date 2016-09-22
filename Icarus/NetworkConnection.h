#pragma once

#pragma comment(lib, "Ws2_32.lib")

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <winsock.h>

class NetworkConnection {

	public:
		NetworkConnection(int connectionID, SOCKET socket);
		~NetworkConnection();
		void write_data();
		void disconnected();

		SOCKET getSocket() {
			return socket;
		};

	private:
		int connectionID;
		SOCKET socket;
		enum { max_length = 1024 };
		char data_read[max_length];
		char data_write[max_length];
};
