/**
 * @file   rfbprotocol.h
 * @author Yifeng Jin <hqking@gmail.com>
 * @date   Tue Feb 14 16:35:07 2012
 * 
 * @brief  rfb protocol header
 * 
 * 
 */
#ifndef __RFBPROTOCOL_H__
#define __RFBPROTOCOL_H__

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned long	u32;
typedef signed char	s8;
typedef signed short	s16;
typedef signed long	s32;

typedef struct {
	u16 r:5;
	u16 g:6;
	u16 b:5;
} PIXEL;

typedef struct {
	u8 bpp;
	u8 depth;
	u8 bigEndian;
	u8 trueColour;
	u16 redMax;		/* big endian */
	u16 greenMax;		/* big endian */
	u16 blueMax;		/* big endian */
	u8 redShift;
	u8 greenShift;
	u8 blueShift;
	u8 pad[3];
} rfbPixelFormat;

#define rfbVersionFormat	"RFB 003.008\n"

/* ****************************************************************
 * security message
 */
enum {
	rfbNone = 1,
	rfbVncAuth = 2,
	rfbRA2 = 5,
	rfbRA2ne = 6,
	rfbTight = 16,
	rfbUltra = 17,
	rfbTLS = 18,
	rfbVeNCrypt = 19,
} rfbSecurityType;

/* ****************************************************************
 * initialization
 */
/* client Init */
typedef struct {
	u8 shared;
} rfbClientInitMsg;

/* server init */
typedef struct {
	u16 framebufferWidth;
	u16 framebufferHeight;
	rfbPixelFormat format;
	u32 nameLength;
	/* u8 *nameString; */
} rfbServerInitMsg;

/* ****************************************************************
 * client to server messages
 */
enum {
	rfbSetPixelFormat = 0,
	rfbSetEncodings = 2,
	rfbFramebufferUpdateRequest = 3,
	rfbKeyEvent = 4,
	rfbPointerEvent = 5,
	rfbClientCutText = 6,
} rfbClientMsgType;

typedef struct {
	u8 type;
	u8 pad[3];
	rfbPixelFormat format;
} rfbSetPixelFormatMsg;

typedef struct {
	u8 type;
	u8 pad;
	u16 number;
	/* s32 encodings */
} rfbSetEncodingsMsg;

typedef struct {
	u8 type;
	u8 incremental;
	u16 x;
	u16 y;
	u16 width;
	u16 height;
} rfbFramebufferUpdateRequestMsg;

typedef struct {
	u8 type;
	u8 down;
	u8 pad[2];
	u32 key;
} rfbKeyEventMsg;

typedef struct {
	u8 type;
	u8 mask;
	u16 x;
	u16 y;
} rfbPointerEventMsg;

typedef struct {
	u8 type;
	u8 pad[3];
	u32 length;
	u8 *text;
} rfbClientCutTextMsg;

/* ****************************************************************
 * server to client messages
 */
enum {
	rfbFramebufferUpdate = 0,
	rfbSetColourMapEntries = 1,
	rfbBell = 2,
	rfbServerCutText = 3,
} rfbServerMsgType;

typedef struct {
	u16 x;
	u16 y;
	u16 width;
	u16 height;
	s32 encoding;
} rfbRectangle;

typedef struct {
	u8 type;
	u8 pad;
	u16 number;
	/* rfbRectangle */
} rfbFramebufferUpdateMsgHdr;

typedef struct {
	u16 red;
	u16 green;
	u16 blue;
} rfbColure;

typedef struct {
	u8 type;
	u8 pad;
	u16 firstColure;
	u16 number;
	/* rfbColure */
} rfbSetColureMapEntriesMsg;

typedef struct {
	u8 type;
} rfbBellMsg;

typedef struct {
	u8 type;
	u8 pad[3];
	u32 length;
	/* text */
} rfbServerCutTextMsg;

/* ****************************************************************
 * encodings
 */
enum {
	ENC_RAW = 0,
	ENC_COPY_RECT = 1,
	ENC_RRE = 2,
	ENC_HEXTILE = 5,
	ENC_ZRLE = 16,
};

typedef struct {
	u16 x;
	u16 y;
} rfbCopyRectEncoding;

typedef struct {
	u32 number;
	PIXEL bgcolour;
} rfbRREencoding;

typedef struct {
	PIXEL bgcolour;
	u16 x;
	u16 y;
	u16 w;
	u16 h;
} rfbRRErectangle;

#endif	/* __RFBPROTOCOL_H__ */
