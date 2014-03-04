/**
 * File: cs_util.h
 *  checksum 
 *
 * author: db
 */

#ifndef __CSUTIL_H
#define __CSUTIL_H

#include "config.h"
#include "types.h"

unsigned short checksum(const void *data, unsigned short len, unsigned int sum);
unsigned short wrapsum(unsigned int sum);

#endif // end of __CSUTIL_H
