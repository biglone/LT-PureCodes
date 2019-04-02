#ifndef _CRC_H_25ED5901_57BF_495a_BA2E_369125E5FF7D_
#define _CRC_H_25ED5901_57BF_495a_BA2E_369125E5FF7D_

#include "common_global.h"

typedef unsigned short      CRC_16_CODE;
typedef unsigned long       CRC_32_CODE;

static const int CRC_16_LENGTH = sizeof(CRC_16_CODE);
static const int CRC_32_LENGTH = sizeof(CRC_32_CODE);

typedef unsigned short      CRC_16_POLY;
typedef unsigned long       CRC_32_POLY;

// 注意：因最高位一定为“1”，故略去 
static const CRC_16_POLY cnCRC_16 = 0x8005; 
// CRC-16 = X16 + X15 + X2 + X0 
static const CRC_16_POLY cnCRC_CCITT = 0x1021; 
// CRC-CCITT = X16 + X12 + X5 + X0，据说这个 16 位 CRC 多项式比上一个要好 
static const CRC_32_POLY cnCRC_32 = 0x04C11DB7; 
// CRC-32 = X32 + X26 + X23 + X22 + X16 + X12 + X11 + X10 + X8 + X7 + X5 + X4 + X2 + X1 + X0 

COMMON_EXPORT CRC_16_CODE CRC_16( unsigned char * aData, unsigned long aSize, CRC_16_POLY aPoly = cnCRC_CCITT );
COMMON_EXPORT CRC_32_CODE CRC_32( unsigned char * aData, unsigned long aSize, CRC_32_POLY aPoly = cnCRC_32 );

#endif//_CRC_H_25ED5901_57BF_495a_BA2E_369125E5FF7D_
