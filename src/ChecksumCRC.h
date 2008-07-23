/*! 
 ***********************************************************************
 * @file      ChecksumCRC.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/07/03
 *
 * This implementation is directly from SAE AS-5669, with slight
 * style modifications.
 ************************************************************************
 */

#ifndef  __CHECKSUM_CRC_H
#define  __CHECKSUM_CRC_H

namespace DeVivo {
namespace Junior {


void crc_accumulate (unsigned char data, unsigned short *crcAccum);
unsigned short crc_calculate (unsigned char *pBuffer, 
                              unsigned short init_value,
                              int length);


}} // namespace DeVivo::Junior



#endif
