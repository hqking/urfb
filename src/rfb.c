/**
 * @file   rfb.c
 * @author Yifeng Jin <hqking@gmail.com>
 * @date   Wed Feb 15 11:43:14 2012
 * 
 * @brief  
 * 
 * 
 */
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "rfb.h"

#define swap16(s) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff))

#define swap24(l) ((((l) & 0xff) << 16) | (((l) >> 16) & 0xff) | (((l) & 0x00ff00)))

#define swap32(l) (((l) >> 24) | (((l) & 0x00ff0000) >> 8) | (((l) & 0x0000ff00) << 8) | ((l) << 24))

#include <stdio.h>

#ifndef RFB_SERVER_NAME
#define RFB_SERVER_NAME "urfb server"
#endif

#ifndef RFB_WIDTH
#define RFB_WIDTH 640
#endif

#ifndef RFB_HEIGHT
#define RFB_HEIGHT 480
#endif

static const char rfbAuthMsg[] = {
	1,
	rfbNone,
};

enum {
	S_CLOSED,
	S_HANDSHAKE,
	S_INITIALIZATION,
	S_MESSAGE,
} gState;

static void rfbError(const char *reason)
{
	int len;

	len = strlen(reason);

	rfbSend((u8 *)&len, sizeof(len));
	rfbSend(reason, len);
}

static void initialization(rfbClientInitMsg *shared)
{
	rfbServerInitMsg serverInfo = {
		.framebufferWidth = swap16(RFB_WIDTH),
		.framebufferHeight = swap16(RFB_HEIGHT),
		.format = {
			.bpp = 16,
			.depth = 16,
			.bigEndian = 0,
			.trueColour = 1,
			.redMax = swap16(31),
			.greenMax = swap16(63),
			.blueMax = swap16(31),
			.redShift = 0,
			.greenShift = 5,
			.blueShift = 11,
		},
		.nameLength = swap32(sizeof(RFB_SERVER_NAME) - 1),
	};

	printf("client initialization message %d\n", shared->shared);

	printf("send server initialization mesage\n");
	rfbSend((u8 *)&serverInfo, sizeof(rfbServerInitMsg));
	rfbSend(RFB_SERVER_NAME, strlen(RFB_SERVER_NAME));

	gState = S_MESSAGE;
}

static void authSuccess(void)
{
	u32 res = 0;

	printf("auth success\n");

	rfbSend((u8 *)&res, sizeof(res));

	gState = S_INITIALIZATION;
}

static void authFail(const char *reason)
{
	u32 res = 1;

	printf("auth fail\n");

	rfbSend((u8 *)&res, sizeof(res));
	rfbError(reason);	

	gState = S_CLOSED;
}

static void authCheck(char type)
{
	int i;

	for (i = 1; i < sizeof(rfbAuthMsg); i++) {
		if (rfbAuthMsg[i] == type)
			break;
	}

	if (i == sizeof(rfbAuthMsg)) {
		authFail("unsupported security type");
		return ;
	}

	switch (type) {
	case rfbNone:
		authSuccess();

		break;
	case rfbVncAuth:
	default:
		assert(0);
	}
}

void rfbBlock(u16 x, u16 y, u16 w, u16 h, u16 colour)
{
	u16 x_swap, y_swap, w_swap, h_swap;

	if (gState != S_MESSAGE)
		return ;

	printf("rfb: send rectangle\n");

	x_swap = swap16(x);
	y_swap = swap16(y);
	w_swap = swap16(w);
	h_swap = swap16(h);

	struct {
		rfbFramebufferUpdateMsgHdr hdr;
		rfbRectangle box;
		rfbRREencoding rre;
		rfbRRErectangle rect;
	} msg = {
		.hdr = {
			.type = rfbFramebufferUpdate,
			.number = swap16(1),
		},
		.box = {
			.x = x_swap,
			.y = y_swap,
			.width = w_swap,
			.height = h_swap,
			.encoding = swap32(ENC_RRE),
		},
		.rre = {
			.number = swap32(1),
			.bgcolour = swap16(colour),
		},
		.rect = {
			.bgcolour = swap16(colour),
			.x = x_swap,
			.y = y_swap,
			.w = w_swap,
			.h = h_swap,
		},
	};

	rfbSend((u8 *)&msg, sizeof(msg));
}

void rfbClose(void)
{
	gState = S_CLOSED;
}

void rfbStart(void)
{
	printf("send rfb version format\n");
	rfbSend(rfbVersionFormat, strlen(rfbVersionFormat));
}

void rfbRecv(u8 *buf, size_t len)
{
	switch (gState) {
	case S_MESSAGE:
		printf("rfb: recv [S_MESSAGE]\n");
		break;

	case S_INITIALIZATION:
		printf("rfb: recv [S_INITIALIZATION]");
		if (len != sizeof(rfbClientInitMsg))
			return ;

		initialization((rfbClientInitMsg *)buf);

		break;

	case S_HANDSHAKE:
		printf("rfb: recv [HANDSHAKE]]\n");
		if (len != 1)
			authFail("invalid handshake response");

		authCheck(*buf);

		break;

	case S_CLOSED:
		printf("rfb: recv [CLOSE]\n");
		if (len != strlen(rfbVersionFormat)) {
			rfbSendByte(0);
			rfbError("invalid rfb version message length");
			return ;
		}

		if (memcmp(rfbVersionFormat, buf, len)) {
			rfbSendByte(0);
			rfbError("invalid rfb version");
			return ;
		}

		printf("rfb: client version is OK, send auth\n");
		rfbSend(rfbAuthMsg, sizeof(rfbAuthMsg));

		gState = S_HANDSHAKE;

		break;
	default:
		assert(0);
	}
}
