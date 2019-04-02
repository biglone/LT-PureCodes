
#include "crc.h"

static const int CRC_TABLE_SIZE = 256;

// 构造 16 位 CRC 表 
void BuildTable16( unsigned long Table_CRC[CRC_TABLE_SIZE], CRC_16_POLY aPoly ) 
{ 
    unsigned short i, j; 
    unsigned short nData; 
    unsigned short nAccum; 

    for ( i = 0; i < CRC_TABLE_SIZE; i++ ) 
    { 
        nData = ( unsigned short )( i << 8 ); 
        nAccum = 0; 
        for ( j = 0; j < 8; j++ ) 
        { 
            if ( ( nData ^ nAccum ) & 0x8000 ) 
                nAccum = ( nAccum << 1 ) ^ aPoly; 
            else 
                nAccum <<= 1; 
            nData <<= 1; 
        } 
        Table_CRC[i] = ( unsigned long )nAccum; 
    } 
} 

// 计算 16 位 CRC 值，CRC-16 或 CRC-CCITT 
CRC_16_CODE CRC_16( unsigned char * aData, unsigned long aSize, CRC_16_POLY aPoly/* = cnCRC_CCITT*/ ) 
{ 
    unsigned long i; 
    unsigned short nAccum = 0; 
    unsigned long Table_CRC[CRC_TABLE_SIZE] = {0};

    BuildTable16( Table_CRC, aPoly );
    for ( i = 0; i < aSize; i++ ) 
        nAccum = ( nAccum << 8 ) ^ ( unsigned short )Table_CRC[( nAccum >> 8 ) ^ *aData++]; 
    return nAccum; 
} 

// 构造 32 位 CRC 表 
void BuildTable32( unsigned long Table_CRC[CRC_TABLE_SIZE], CRC_32_POLY aPoly ) 
{ 
    unsigned long i, j; 
    unsigned long nData; 
    unsigned long nAccum; 

    for ( i = 0; i < CRC_TABLE_SIZE; i++ ) 
    { 
        nData = ( unsigned long )( i << 24 ); 
        nAccum = 0; 
        for ( j = 0; j < 8; j++ ) 
        { 
            if ( ( nData ^ nAccum ) & 0x80000000 ) 
                nAccum = ( nAccum << 1 ) ^ aPoly; 
            else 
                nAccum <<= 1; 
            nData <<= 1; 
        } 
        Table_CRC[i] = nAccum; 
    } 
} 

// 计算 32 位 CRC-32 值 
CRC_32_CODE CRC_32( unsigned char * aData, unsigned long aSize, CRC_32_POLY aPoly/* = cnCRC_32*/ ) 
{ 
    unsigned long i; 
    unsigned long nAccum = 0; 
    unsigned long Table_CRC[CRC_TABLE_SIZE] = {0};

    BuildTable32( Table_CRC, aPoly ); 
    for ( i = 0; i < aSize; i++ ) 
        nAccum = ( nAccum << 8 ) ^ Table_CRC[( nAccum >> 24 ) ^ *aData++]; 
    return nAccum; 
} 
