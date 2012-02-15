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
#include "proto.h"

#ifndef RFB_SERVER_NAME
#define RFB_SERVER_NAME "urfb server"
#endif

#ifndef RFB_WIDTH
#define RFB_WIDTH 640
#endif

#ifndef RFB_HEIGHT
#define RFB_HEIGHT 480
#endif

extern void rfbSend(const u8 *data, size_t len);
extern void rfbSendByte(u8 data);

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
		.framebufferWidth = RFB_WIDTH,
		.framebufferHeight = RFB_HEIGHT,
		.format = {
			.bpp = 16,
			.depth = 16,
			.bigEndian = 0,
			.trueColour = 1,
			.redMax = 31,
			.greenMax = 63,
			.blueMax = 31,
			.redShift = 0,
			.greenShift = 5,
			.blueShift = 11,
		},
		.nameLength = sizeof(RFB_SERVER_NAME),
		.nameString = RFB_SERVER_NAME,
	};

	rfbSend((u8 *)&serverInfo, sizeof(serverInfo));

	gState = S_MESSAGE;
}

static void authSuccess(void)
{
	u32 res = 0;

	rfbSend((u8 *)&res, sizeof(res));

	gState = S_INITIALIZATION;
}

static void authFail(const char *reason)
{
	u32 res = 1;

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
	rfbFramebufferUpdateMsg msg = {
		.type = rfbFramebufferUpdate,
		.number = 1,
	};

	rfbRectangle box = {
		.x = x,
		.y = y,
		.width = w,
		.height = h,
		.encoding = ENC_RRE,
	};

	rfbRREencoding rre = {
		.number = 1,
		.bgcolour = colour,
	};

	rfbRRErectangle rect = {
		.bgcolour = colour,
		.x = x,
		.y = y,
		.w = w,
		.h = h,
	};

	rfbSend((u8 *)&msg, sizeof(msg));
	rfbSend((u8 *)&box, sizeof(box));
	rfbSend((u8 *)&rre, sizeof(rre));
	rfbSend((u8 *)&rect, sizeof(rect));
}

void rfbClose(void)
{
	gState = S_CLOSED;
}

void rfbStart(void)
{
	rfbSend(rfbVersionFormat, strlen(rfbVersionFormat));
}

void rfbRecv(u8 *buf, size_t len)
{
	switch (gState) {
	case S_MESSAGE:
		break;

	case S_INITIALIZATION:
		if (len != sizeof(rfbClientInitMsg))
			return ;

		initialization((rfbClientInitMsg *)buf);

		break;

	case S_HANDSHAKE:
		if (len != 1)
			authFail("invalid handshake response");

		authCheck(*buf);

		break;

	case S_CLOSED:
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

		rfbSend(rfbAuthMsg, sizeof(rfbAuthMsg));

		gState = S_HANDSHAKE;

		break;
	default:
		assert(0);
	}
}
