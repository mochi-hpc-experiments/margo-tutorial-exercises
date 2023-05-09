#ifndef PARAM_H
#define PARAM_H

#include <mercury.h>
#include <mercury_macros.h>

/* We use the Mercury macros to define the input
 * and output structures along with the serialization
 * functions.
 */
MERCURY_GEN_PROC(sum_in_t,
        ((int32_t)(x))\
        ((int32_t)(y)))

MERCURY_GEN_PROC(sum_out_t, ((int32_t)(ret)))

/* (4) add the definition of the insert_in/out_t and lookup_in/out_t types
 * using MERCURY_GEN_PROC. insert_in_t will need a name (hg_string_t) and
 * a number. insert_out_t can simply contain a uint32_t error code.
 * lookup_in_t will need a name. lookup_out_t will need a number and an
 * error code. Note that using hg_string_t requires including
 * mercury_proc_string.h.
 */

#endif
