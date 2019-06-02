/* Copyright (c) 2019 Andy Holland <github@ahweb.co.uk> Licence GPL */
/* Some apects of API design are modelled after PHP's Sodium extension under BSD 2-Clause Simplified Licence */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <new>
#include <string>

#include "sodium_udf.h"

#ifndef __PACKED__
# if defined(__INTEL_COMPILER) || defined(_MSC_VER)
#  define __PACKED__ __declspec(align(1))
# else
#  define __PACKED__ __attribute__ ((__packed__))
# endif
#endif

struct __PACKED__ Buffer {
    /** Maximum length available in this buffer, including the final, reserved, null byte. */
    size_t  length;
    /** Each buffer is over-allocated such that string is really char[length]. */
    char    string[1];
};

#define ALLOC_FOR_BUFFER(capacity)  (sizeof(Buffer) + capacity)

#define SIZEOF_BUFFER(B)            (sizeof(size_t) + B->length)


// Used in the macro fixed_buffer
char *dynamic_buffer(char *preallocated, size_t required, char **store) {
    Buffer *buffer;

    if (required < mysql_RESULT_LENGTH)  {
        return preallocated;
    }

    if (*store != NULL) {
        buffer = (Buffer *)*store;

        if (required < buffer->length) {
            Sodium::sodium_memzero(buffer->string, buffer->length);
            return buffer->string;
        }

        Sodium::sodium_memzero(buffer, SIZEOF_BUFFER(buffer));
        Sodium::sodium_free(buffer);
    }

    // Round (required + 1) up to a multiple of 512 bytes
    required = ((1 + (required >> 9)) << 9);

    buffer = (Buffer *)Sodium::sodium_malloc(ALLOC_FOR_BUFFER(required));
    buffer->length = required;
    Sodium::sodium_memzero(buffer->string, required);

    *store = (char*)buffer;

    return buffer->string;
}

/** Free the Buffer which contains string. */
void free_buffer(char *string) {
    void * const buffer = string - sizeof(size_t);
    Sodium::sodium_free(buffer);
}

#include "sodium_auth.cc"
#include "sodium_box.cc"
#include "sodium_hash.cc"
#include "sodium_kdf.cc"
#include "sodium_kx.cc"
#include "sodium_pad.cc"
#include "sodium_pwhash.cc"
#include "sodium_secretbox.cc"
#include "sodium_sign.cc"

#ifndef MYSQL_SODIUM_NO_AGGREGATE
#include "group_hash.cc"
#endif