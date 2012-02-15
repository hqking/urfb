/**
 * @file   tcp.c
 * @author Yifeng Jin <hqking@gmail.com>
 * @date   Tue Feb 14 21:05:21 2012
 * 
 * @brief  tcp socket as rfb link layer 
 * 
 * 
 */
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "proto.h"

static int gClient;

void rfbSend(const u8 *data, size_t len)
{
	send(gClient, data, len, 0);
}

void rfbSendByte(u8 data)
{
	send(gClient, &data, sizeof(data), 0);
}

int serverStart(void)
{
	struct sockaddr_in addr, client;
	int sock;
	socklen_t len;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(sock, (struct sockaddr *)&addr, sizeof(addr));

	listen(sock, 10);

	len = sizeof(client);
	return accept(sock, (struct sockaddr *)&client, &len);
}

static u8 data[128];
int main(void)
{
	int rc;
	int x = 0, y = 0;

	gClient = serverStart();

	rfbStart();

	while (1) {
		rc = recv(gClient, data, sizeof(data), MSG_DONTWAIT);
		if (rc > 0)
			rfbRecv(data, rc);
		else if (rc == 0)
			break;
		else 
			exit(-1);
		

		sleep(1);
		rfbBlock(50 + x++, 50 + y++, 100, 100, 0x9999);
		x %= 200;
		y %= 200;
	}
}
