/*
 * Copyright (c) 2025-Present, Davide Usberti <usbertibox@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _TOONC_H
#define _TOONC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* List markers */
#define LIST_ITEM_MARKER (2 << 0)
#define LIST_ITEM_PREFIX (2 << 1)

/* Structural characters */
#define COMMA   (2 << 2)
#define COLON   (2 << 3)
#define SPACE   (2 << 4)
#define PIPE    (2 << 5)
#define DOT     (2 << 6)

char STRUCT_CHARS[] = {
    [COMMA] = ',',
    [COLON] = ':',
    [SPACE] = ' ',
    [PIPE]  = '|',
    [DOT]   = '.'
};

/* Brackets and braces */
#define OPEN_BRACKET    (2 << 7)
#define CLOSE_BRACKET   (2 << 8)
#define OPEN_BRACE      (2 << 9)
#define CLOSE_BRACE     (2 << 10)

/* Escape characters */
#define BACKSLASH       (2 << 11)
#define DOUBLE_QUOTE    (2 << 12)
#define NEWLINE         (2 << 13)
#define CARRIAGE_RETURN (2 << 14)
#define TAB             (2 << 15)

/* Error severity */
#define ERR_ERR     (2 << 0)
#define ERR_WARN    (2 << 1)
#define ERR_FATAL   (2 << 2)

/* ===================== Key-value types ======================*/
#define KV_STRING 0 /* i.e. name: Bob */
#define KV_INT    1 /* i.e. age: 21 */
#define KV_BOOL   2 /* i.e. married: false */
#define KV_NULL   3 /* i.e. `job: ` */
#define KV_DOUBLE 4 /* i.e. height: 180.42 */

/* KW_OBJ - Object value type
 *
 * car:
 *     model: Fiat panda 4x4
 *     year: 2014 
 */
#define KV_OBJ    5 
#define KV_LIST   6 /* i.e. nicknames[3]: bobby,bub,bibi */
/* KW_LOBJ - List object value type 
 *
 * JSON:
 *  {
 *      "children": [
 *          { "name": "Alice", "age": 12 },
 *          { "name": "Luke", "age": 20 }
 *      ] 
 *  }
 *
 * TOON:
 * children[2]{name,age}:
 *          Alice,12
 *          Luke,20
 */
#define KV_LOBJ   7 

/* ======================= Data Structures ======================= */

struct toonStr {
    char *ptr;
    size_t len;
};

typedef struct toonObject {
    int kvtype; /* KV_STRING, KV_OBJ ... */
    int indent; /* Indentation level */
    char *key; /* property name */
    union {
        struct toonStr str;
        int i;
        double d;
        int boolean:1;
        
        /* Array data type */
        struct {
            struct toonObject **items;
            size_t len;
            size_t capacity;
        } array;
    };

    struct toonObject *child; /* for recursive types */
    struct toonObject *next; /* next object (linked list) */
} toonObject;

/* Minimal toonParser object */
typedef struct toonParser {
    char *source; /* the original text */
    char *p; /* current cursor position */
    int line; /* current line numbner for error checking */
} toonParser;

/* Rapresentation of the parsing error */
typedef struct {
    char *message; 
    int line; 
    int column; 
    int severity; /* error type (ERR_ERR, ERR_WARN, ...) */
} parseError;

#ifdef __cplusplus
}
#endif

#endif /* _TOONC_H */
