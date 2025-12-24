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
 *
 * ================================= Program Notes ========================================
 * This C library provides an easy-to-use API to parse the TOON 
 * Token-Oriented Object Notation is a compact, human-readable encoding of the JSON 
 * data model for LLM prompts. It provides a lossless serialization of the same 
 * objects, arrays, and primitives as JSON, but in a syntax that minimizes tokens 
 * and makes structure easy for models to follow.
 *
 * TOON combines YAML's indentation-based structure for nested objects with a CSV-style
 * tabular layout for uniform arrays. TOON's sweet spot is uniform arrays of objects
 * (multiple fields per row, same structure across items), achieving CSV-like
 * compactness while adding explicit structure that helps LLMs parse and validate 
 * data reliably. For deeply nested or non-uniform data, JSON may be more efficient.
 *
 * The similarity to CSV is intentional: CSV is simple and ubiquitous, 
 * and TOON aims to keep that familiarity while remaining a lossless, drop-in 
 * representation of JSON for Large Language Models.
 *
 * Think of it as a translation layer: use JSON programmatically, 
 * and encode it as TOON for LLM input.
 */

/* ============================ Compiler optimization macros ============================== */
#include "toonc.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>

#if defined(_MSC_VER)
    /* Microsoft Visual C++ */
    #define FORCE_INLINE __forceinline
    #define NO_INLINE    __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
    /* GCC, Clang */
    #define FORCE_INLINE __attribute__((always_inline))
    #define NO_INLINE    __attribute__((noinline))
#else
    #define FORCE_INLINE inline
    #define NO_INLINE
#endif

#if defined(__GNUC__) || defined(__clang__)
    /* GCC, Clang */
    #define LIKELY(x)   __builtin_expect(!!(x), 1)
    #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    /* MSVC and others*/
    #define LIKELY(x)   (x)
    #define UNLIKELY(x) (x)
#endif

#if defined(_MSC_VER)
    /* Microsoft Visual C++ */
    #define ALIGN(N) __declspec(align(N))
#elif defined(__GNUC__) || defined(__clang__)
    /* GCC, Clang */
    #define ALIGN(N) __attribute__((aligned(N)))
#elif __STDC_VERSION__ >= 201112L || (defined(__cplusplus) && __cplusplus >= 201103L)
    /* Standard C11 or C++11 */
    #define ALIGN(N) _Alignas(N)
#else
    /* Fallback standard */ 
    #define ALIGN(N)
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define UNUSED __attribute__((unused))
#else
    #define UNUSED
#endif

/* ============================== Allocation wrappers ============================= */

/* This helper checks for overflow in multiplication size * nmemb */
static int safe_mul_size(size_t nmemb, size_t size, size_t *result) {
        if (size != 0 && nmemb > SIZE_MAX / size) {
                return 0; // overflow
        }
        *result = nmemb * size;
        return 1;
}

/* safe implementation for malloc */
void *tmalloc(size_t size) {
    if (UNLIKELY(size == 0)) size = 1; /* in case of malloc(0) */
    void *ptr = malloc(size);
    
    if (UNLIKELY(ptr == NULL)) {
        fprintf(stderr, "%s\n", "TOONC: memory allocation failed.");
    }
    
    return ptr;
}

void *tcalloc(size_t nmemb, size_t size) {
    size_t total;
    if (!safe_mul_size(nmemb, size, &total)) {
        fprintf(stderr, "%s\n", "memory allocation failed.");
    }
    
    void *ptr = calloc(1, total);
    if (UNLIKELY(ptr == NULL)) {
        fprintf(stderr, "%s\n", "memory allocation failed.");
    }
    
    return ptr;
}

void *trealloc(void *ptr, size_t size) {
    if (UNLIKELY(size == 0)) size = 1;
    void *new_ptr = realloc(ptr, size);
    if (UNLIKELY(new_ptr == NULL)) {
        fprintf(stderr, "%s\n", "memory allocation failed.");
    }

    return new_ptr;
}

void tfree(void **ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL; /* avoid dangling pointer */
    }
}

/* ============================ Object related functions ========================= */

/* Crate a new TOON object by given key-value type */
toonObject *newObject(int kvtype) {
    toonObject *o = tmalloc(sizeof(toonObject));
    memset(o, 0, sizeof(*o));
    o->kvtype = kvtype;
    o->key = NULL;
    o->indent = 0;
    o->next = NULL;
    o->child = NULL;
    return o;
}

/* Create a new string object, safe allocating the string */
toonObject *newStringObj(char *s, size_t len) {
    toonObject *o = newObject(KV_STRING);
    o->str.ptr = tmalloc(len + 1);
    o->str.len = len;
    memcpy(o->str.ptr, s, len);
    o->str.ptr[len] = '\0';
    return o;
}

/* Create a new integer object */
toonObject *newIntObj(int value) {
    toonObject *o = newObject(KV_INT);
    o->i = value;
    return o;
}

/* Create a new double precision floating point object */
toonObject *newDoubleObj(double value) {
    toonObject *o = newObject(KV_DOUBLE);
    o->d = value;
    return o;
}

/* Create a new boolean object by given integer (boolean) */
toonObject *newBoolObj(int value) {
    toonObject *o = newObject(KV_BOOL);
    o->boolean = !!value;
    return o;
}

/* Create a new null object (no value) */
toonObject *newNullObj(void) {
    return newObject(KV_NULL);
}

/* Create a new list/array object */
toonObject *newListObj(void) {
    toonObject *o = newObject(KV_LIST);
    o->array.items = NULL;
    o->array.len = 0;
    o->array.capacity = 0;
    return o;
}

/* Push an item to a list object, It also handles the capacity. */
void listPush(toonObject *list, toonObject *item) {
    size_t len = list->array.len;
    size_t capacity = list->array.capacity;

    /* Resize the array of N * 2 each time is full */
    if (UNLIKELY(len >= capacity)) {
        size_t new_cap = capacity == 0 ? 4 : capacity * 2;
        list->array.items = trealloc(list->array.items, sizeof(toonObject) * new_cap);
        list->array.capacity = new_cap;
    }
    list->array.items[list->array.len++] = item;
}

/* ================================ Parsing functions ============================= */

/* Skip all the whitespace in a row */
FORCE_INLINE void parseSpaces(toonParser *parser) {
    while (parser->p[0] == ' ' || parser->p[0] == '\t')
        parser->p++;
}

/* Parse the new line char */
FORCE_INLINE void parseNewLine(toonParser *parser) {
    if (LIKELY(parser->p[0] == '\n')) {
        parser->p++;
        parser->line++;
    }
}

/* Parse the initial indentation spaces. (2 spaces = 1 indentation level)
 * ATTENTION: It only works with 2 spaces, tab is not supported yet
 * ATTENTION: In case of odd indentation the result will be wrong */
FORCE_INLINE int parseIndent(toonParser *parser) {
    int spaces = 0;
    while (parser->p[0] == ' ') {
        spaces++;
        parser->p++;
    }
    
    return spaces / 2;
}

/* Parse the object key  */
char *parseKey(toonParser *parser, size_t *len) {
    char *start = parser->p;

    /* Read line until :, [, \n */
    while (parser->p[0] &&
            parser->p[0] != ':' &&
            parser->p[0] != '[' &&
            parser->p[0] != '\n') {
        parser->p++;
    }

    char *end = parser->p;

    /* trim trailing white spaces */
    while (end > start && isspace(*(end - 1))) 
        end--;

    *len = end - start;
    return start;
}

/* get the array/list size inside the key[...] */
int parseArraySize(toonParser *parser) {
    /* Skip array notation like [5] and return the size */
    int size = -1;
    if (parser->p[0] == '[') {
        parser->p++;
        if (isdigit(parser->p[0])) {
            size = atoi(parser->p);
            while (isdigit(parser->p[0])) parser->p++;
        }
        if (parser->p[0] == ']') 
            parser->p++;
    }
    return size;
}

/* Check if a given string is a number or a floating point */
int isNumber(char *s, size_t len, int *is_float) {
    if (len == 0) return 0;

    int i = 0;
    *is_float = 0;
    
    /* handle negative numbers */
    if (s[i] == '-') i++;

    if (i >= len || !isdigit(s[i])) return 0;

    /* parse digits */
    while (i < len && isdigit(s[i])) i++;

    /* check for decimal point */
    if (i < len && s[i] == '.') {
        *is_float = 1;
        i++;
        if (i >= len || isdigit(s[i])) return 0;
        while (i < len && isdigit(s[i])) i++;
    }

    return i == len;
}

/* Parse the value and retrun the parsed object */
toonObject *parseValue(toonParser *parser) {
    parseSpaces(parser);

    /* Empty values means nested object */
    if (parser->p[0] == '\n' || parser->p[0] == '\0') {
        return NULL;
    }

    char *start = parser->p;

    /* Read untile new line or comma (for arrays) */
    while (parser->p[0] && parser->p[0] != '\n' && parser->p[0] == ',') 
        parser->p++;

    char *end = parser->p;

    /* trim white-spaces */
    while (start < end && isspace(*start)) start++;
    while (end > start && isspace(*(end-1))) end--;
    
    size_t len = end - start;
    if (len == 0) return newNullObj();

    /* Handle and remove quotes */
    int quotes = 0;
    if (len >= 2 && *start == '"' && *(end - 1) == '"') {
        start++;
        end--;
        len -= 2;
        quotes = 1;
    }
    
    /* in case the value contains quotes is a string
     * so return a new string object. */
    if (quotes) 
        return newStringObj(start, len);

    /* check for boolean value */
    if (len == 4 && strncmp(start, "true", 4) == 0) {
        return newBoolObj(true);
    }
    if (len == 5 && strncmp(start, "false", 5) == 0) {
        return newBoolObj(false);
    }
    
    /* check for null value */
    if (len == 4 && strncmp(start, "null", 4) == 0) {
        return newNullObj();
    }
    
    /* check for double or integer value */
    int is_float;
    if (isNumber(start, len, &is_float)) {
        char buf[128];
        if (len < sizeof(buf)) {
            memcpy(buf, start, len);
            buf[len] = '\0';

            return is_float ? newDoubleObj(atof(buf)) 
                : newIntObj(atoi(buf));
        }
    }
    
    /* default value type: string */
    return newStringObj(start, len);
}

/* Parse array value i.e. item1,item2,item3 */
toonObject *parseListValues(toonParser *parser) {
    toonObject *arr = newListObj();

    while (parser->p[0] && parser->p[0] != '\n') {
        toonObject *item = parseValue(parser);
        if (item) listPush(arr, item);
        
        /* skip comma */
        if (LIKELY(parser->p[0] == ',')) {
            parser->p++;
        } else {
            break;
        }
    }

    return arr;
}

/* Check if the line is a comment or is empty  */
FORCE_INLINE int isCommentOrEmpty(toonParser *parser) {
    parseSpaces(parser);
    return parser->p[0] == '#' || parser->p[0] == '\n' || parser->p[0] == '\0';
}

/* Skip an entire line until \n */
FORCE_INLINE void skipLine(toonParser *parser) {
    while (parser->p[0] && parser->p[0] != '\n') 
        parser->p++;
    parseNewLine(parser);
}

/* ============================== Parsing functions ============================ */

/* The general parsing function for TOONc, this function checks for know
 * parsing pattern and converts them into toonObjects. */
toonObject *parse(char *source) {
    toonParser parser;
    parser.source = source;
    parser.p = source;
    parser.line = 1;

    toonObject *root = newObject(KV_OBJ);
    toonObject *current = NULL;

    while (parser.p[0]) {
        /* This checks if the line is presented by a
         * comment or a blank line, and skips it */
        if (UNLIKELY(isCommentOrEmpty(&parser))) {
            if (parser.p[0] == '#' || parser.p[0] == '\n') {
                skipLine(&parser);
            }
            continue;
        }
        
        /* Parse the indentation value */
        int indent = parseIndent(&parser);
        
        /* Parse key */
        size_t keylen;
        char *key = parseKey(&parser, &keylen);
        if (keylen == 0) {
            skipLine(&parser);
            continue;
        }
    
        /* Check for array notation */
        int array_size = parseArraySize(&parser);

        if (parser.p[0] != ':') {
            fprintf(stderr, "Syntax error at line %d: expected ':'\n", parser.line);
            skipLine(&parser);
            continue;
        }
        parser.p++; /* skip the ':' */
       
        /* property object */
        toonObject *prop;
        prop = array_size >= 0 ? parseListValues(&parser)
            : parseValue(&parser);

        /* Set key and indentation */
        if (prop) {
            prop->key = tmalloc(keylen + 1);
            memcpy(prop->key, key, keylen);
            prop->key[keylen] = 0;
            prop->indent = indent;
        } else {
            /* In case of no values it means the object
             * is a nested object */
            prop = newObject(KV_OBJ);
            prop->key = tmalloc(keylen + 1);
            memcpy(prop->key, key, keylen);
            prop->key[keylen] = 0;
            prop->indent = indent;
        }

        if (current == NULL) {
            root->child = current = prop;
        } else {
            current->next = prop;
            current = prop;
        }

        parseNewLine(&parser);
    }

    return root;

}

/* ====================== Utility functions =================== */

void printObject(toonObject *o, int depth) {
    if (o == NULL) return;

    for (int i = 0; i < depth; i++) printf("  ");

    if (o->key) printf("%s: ", o->key);

    switch (o->kvtype) {
    case KV_STRING:
        printf("\"%s\" (string)", o->str.ptr);
        break;
    case KV_INT:
        printf("%d (integer)", o->i);
        break;
    case KV_DOUBLE:
        printf("%f (double)", o->d);
        break;
    case KV_BOOL:
        printf("%s (boolean)", o->boolean ? "true" : "false");
        break;
    case KV_NULL:
        printf("null (null)");
        break;
    case KV_LIST:
        printf("[");
        for (size_t i = 0; i < o->array.len; i++) {
            toonObject *item = o->array.items[i];

            switch (item->kvtype) {
                case KV_STRING:
                    printf("\"%s\"", item->str.ptr);
                    break;
                case KV_INT:
                    printf("%d", item->i);
                    break;
                case KV_DOUBLE:
                    printf("%f", item->d);
                    break;
                case KV_BOOL:
                    printf("%s", item->boolean ? "true" : "false");
                    break;
                default:
                    printf("?");
            }
            if (i < o->array.len - 1) printf(", ");
        }
        printf("] (array)");
        break;
    case KV_OBJ:
        printf("{ (object)");
        break;
    }
    printf("\n");

    if (o->child) 
        printObject(o->child, depth + 1);

    if (o->kvtype == KV_OBJ && depth > 0) {
        for (int i = 0; i < depth; i++) printf("  ");
        printf("}\n");
    }
    
    if (o->next) {
        printObject(o->next, depth);
    }
}

void printRoot(toonObject *root) {
    if (root && root->child) {
        printObject(root->child, 0);
    }
}

#if 0

int main(int argc, char **argv) {
    printf("Hello, TOONc!\n");
    return 0;
}

#endif
