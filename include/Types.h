/*! 
 ***********************************************************************
 * @file      Types.h
 * @author    Dave Martin, DeVivo AST, Inc.  
 * @date      2008/03/03
 *
 * @attention Copyright (C) 2008
 * @attention DeVivo AST, Inc.
 * @attention All rights reserved
 ************************************************************************
 */
#ifndef __COMMON_TYPES_H
#define __COMMON_TYPES_H

#include <string>
#include <list>

namespace DeVivo {
namespace Junior {

static unsigned char getByte(unsigned long in, char num)
{
    return ((unsigned char)(in>>(num*8)));
}


// Types for JAUS_ID.  The JAUS ID is simply an unsigned long
// but has to watch out for wildcard bytes (0xFF) during
// comparison operations.
class JAUS_ID
{
  public:
    JAUS_ID(){val=0;};
    JAUS_ID(unsigned long in){val=in;}
    ~JAUS_ID(){}

    unsigned long val;
    bool operator==(const JAUS_ID& in) const
    {
        // Check for the easy case for computation efficiency.
        if (val == in.val) return true;

        // Each byte may have a wildcard (0xFF), so we need to check bytewise
        // comparisons.
        for (char i=0; i<4; i++)
            if ((getByte(val, i) != 0xFF) &&
                (getByte(in.val, i) != 0xFF) &&
                (getByte(val, i) != getByte(in.val, i)))
            {
                return false;
            }

        // Getting to this point means each byte is equivalent or
        // a wildcard.
        return true;
    }
    bool operator<(const JAUS_ID& in) const
    {
        if (val < in.val) return true;
        return false;
    }
    bool operator!=(const JAUS_ID& in) const
    {
        if (val != in.val) return true;
        return false;
    }
    bool containsWildcards()
    {
        // Each byte may have a wildcard (0xFF), so we need to check each
        for (char i=0; i<4; i++)
            if (getByte(val, i) == 0xFF) return true;
        return false;
    }
};
}} // namespace DeVivo::Junior
#endif



