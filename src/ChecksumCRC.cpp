/*! 
 ***********************************************************************
 * @file      ChecksumCRC.cpp
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/07/03
 *
 * This implementation is directly from SAE AS-5669, with slight
 * style modifications.
 ************************************************************************
 */

#include "ChecksumCRC.h"


void DeVivo::Junior::crc_accumulate (unsigned char data, unsigned short *crcAccum) 
{
    //  Accumulate one byte of data into the CRC.
    unsigned char tmp;
        
    tmp = data ^ (unsigned char)(*crcAccum & 0xff);
    tmp ^= (tmp << 4);
    *crcAccum = (*crcAccum >> 8) ^ (tmp << 8)
              ^ (tmp << 3) ^ (tmp >> 4);
}

unsigned short DeVivo::Junior::crc_calculate (unsigned char *pBuffer, 
                                              unsigned short init_value,
                                              int length)
{
    unsigned short crcTmp = init_value;
    unsigned char *pTmp = pBuffer;

    //  For a "message" of length bytes contained in the unsigned character
    //  array pointed to by pBuffer, calculate the CRC.
    for (int i=0; i<length; i++)
        crc_accumulate(*pTmp++, &crcTmp);
    return crcTmp;      
}
