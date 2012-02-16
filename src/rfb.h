/**
 * @file   rfb.h
 * @author Yifeng Jin <hqking@gmail.com>
 * @date   Thu Feb 16 16:04:20 2012
 * 
 * @brief  
 * 
 * 
 */
#ifndef __RFB_H__
#define __RFB_H__

#include "rfbprotocol.h"


extern void rfbSend(const u8 *data, size_t len);
extern void rfbSendByte(u8 data);
void rfbRecv(u8 *buf, size_t len);

void rfbStart(void);
void rfbClose(void);

#endif	/* __RFB_H__ */
