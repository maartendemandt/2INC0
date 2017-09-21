/* 
 * Operating Systems [2INCO] Practical Assignment
 * Interprocess Communication
 *
 * Contains definitions which are commonly used by the farmer and the workers
 *
 */

#ifndef COMMON_H
#define COMMON_H


#include "uint128.h"

// maximum size for any message in the tests
#define MAX_MESSAGE_LENGTH	6
 

// TODO: put your definitions of the datastructures here
typedef struct
{
    // a data structure with 3 members
    char                    start_char;
    char                    end_char;
    int                     length;
    uint128_t               md5;
} MQ_REQUEST_MESSAGE;

typedef struct
{
    // a data structure with 3 members
    uint128_t               md5;
} MQ_RESPONSE_MESSAGE;

#endif

