/* The Connection class */

#include <cstdio>
#include <WinSock2.h>

#include "connection.h"

#pragma comment(lib, "ws2_32.lib")


// Constructor.
// sock: the socket that we've accepted the client connection on.
Connection::Connection(SOCKET sock)
	: sock_(sock), readCount_(0), writeCount_(0), reading(true), writing(false)
{
	printf("New connection\n");
}

// Destructor.
Connection::~Connection()
{
	printf("Closing connection\n");
	closesocket(sock_);
}

// Return the client's socket.
SOCKET Connection::sock()
{
	return sock_;
}

// Return whether this connection is in a state where we want to try
// reading from the socket.
int Connection::wantRead()
{
	return readCount_;
}

int Connection::wantWrite()
{
	return writeCount_;
}

// Call this when the socket is ready to read.
bool Connection::doRead()
{
	reading = true;

	// Receive as much data from the client as will fit in the buffer.
	int spaceLeft = (sizeof readBuffer_) - readCount_;
	int count = recv(sock_, readBuffer_ + readCount_, spaceLeft, 0);

	if (count <= 0)
	{
		printf("Client connection closed or broken\n");
		return true;
	}

	// We've successfully read some more data into the buffer.
	readCount_ += count;

	if (readCount_ < MESSAGESIZE) {
		// ... but we've not received a complete message yet.
		// So we can't do anything until we receive some more.
		return false;
	}

	// We've got a complete message.
	printf("Received message from the client: '");
	fwrite(readBuffer_, 1, MESSAGESIZE, stdout);
	printf("'\n");

	if (memcmp(readBuffer_, "quit", 4) == 0)
	{
		printf("Client asked to quit\n");
		return true;
	}

	// Copy the data that we have read from the read buffer into the write buffer ready for echo
	memcpy(writeBuffer_, readBuffer_, min(readCount_, MESSAGESIZE));
	writeCount_ = readCount_;

	// Clear the buffer, ready for the next message.
	readCount_ = 0;

	// We have finished reading
	reading = false;
	return false;
}

bool Connection::doWrite()
{
	writing = true;

	// Send the same data back to the client.
	// FIXME: the socket might not be ready to send yet -- so this could block!
	// FIXME: and we might not be able to write the entire message in one go...

	// Add code to write/echo to client
	int messageSize = min(writeCount_, MESSAGESIZE);
	int count = send(sock_, writeBuffer_, messageSize, 0);
	int writeLeft = (sizeof writeBuffer_) - writeCount_;

	if (count == SOCKET_ERROR)
	{
		printf("Error sending echo\n");
		return true;
	}
	
	if (count == writeCount_)
	{
		printf("Send successful\n");
		// We managed to send all the msg so clear the writeCount_
		writeCount_ = 0;
	}
	else
	{
		// We must of only sent part of the msg, remove that data from the buffer
		writeCount_ -= count;

		// Overwrite the existing writeBuffer
		//memmove(writeBuffer_, writeBuffer_ + count, writeCount_);


		printf("send failed\n");
		return true;
	}

	writing = false;
	return false;
}
