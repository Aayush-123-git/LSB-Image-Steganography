#ifndef TYPES_H
#define TYPES_H

typedef unsigned int uint;

/* Status of function calls */
typedef enum
{
    e_success,
    e_failure
} Status;

/* Operation type */
typedef enum
{
    e_encode,
    e_decode,
    e_unsupported
} OperationType;''

#endif