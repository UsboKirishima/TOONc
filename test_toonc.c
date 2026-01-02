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
 * ===============================================================================
 *
 * TOONc Test Suite 
 * 
 * Comprehensive test suite for the TOON (Token-Oriented Object Notation) parser.
 * Tests all major functionality including parsing, querying, type handling,
 * memory management, and edge cases.
 *
 * Test Categories:
 * - Basic parsing (primitives, strings, numbers)
 * - Nested object structures
 * - Array handling (simple and tabular)
 * - Comments and whitespace
 * - Edge cases and error recovery
 * - Memory management
 * - Type checking
 * - Complex real-world scenarios
 * - File I/O
 * - Performance benchmarks
 */

#include "toonc.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* ============================================================================
 * Test Framework Macros
 * ========================================================================== */

/* ANSI color codes for better output readability */
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

/* Test lifecycle macros */
#define TEST_BEGIN(name) \
    do { \
        printf("\n" COLOR_CYAN COLOR_BOLD "=== Test: %s ===" COLOR_RESET "\n", name); \
    } while(0)

#define TEST_END(name) \
    do { \
        printf(COLOR_GREEN "✓ %s passed" COLOR_RESET "\n", name); \
    } while(0)

/* Assertion macros with detailed error reporting */
#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, COLOR_RED COLOR_BOLD "✗ Assertion failed" COLOR_RESET "\n"); \
            fprintf(stderr, "  Location: %s:%d\n", __FILE__, __LINE__); \
            fprintf(stderr, "  Condition: %s\n", #cond); \
            return 1; \
        } \
    } while(0)

#define ASSERT_MSG(cond, msg, ...) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, COLOR_RED COLOR_BOLD "✗ Assertion failed" COLOR_RESET "\n"); \
            fprintf(stderr, "  Location: %s:%d\n", __FILE__, __LINE__); \
            fprintf(stderr, "  Condition: %s\n", #cond); \
            fprintf(stderr, "  Message: " msg "\n", ##__VA_ARGS__); \
            return 1; \
        } \
    } while(0)

/* Type-specific assertions */
#define ASSERT_NOT_NULL(ptr) \
    ASSERT_MSG(ptr != NULL, "Expected non-NULL pointer")

#define ASSERT_NULL(ptr) \
    ASSERT_MSG(ptr == NULL, "Expected NULL pointer")

#define ASSERT_EQ(a, b) \
    ASSERT_MSG((a) == (b), "Expected %d, got %d", (int)(b), (int)(a))

#define ASSERT_STR_EQ(a, b) \
    do { \
        const char *_a = (a); \
        const char *_b = (b); \
        if (_a == NULL || _b == NULL) { \
            fprintf(stderr, COLOR_RED COLOR_BOLD "✗ Assertion failed" COLOR_RESET "\n"); \
            fprintf(stderr, "  Location: %s:%d\n", __FILE__, __LINE__); \
            fprintf(stderr, "  Expected: \"%s\"\n", _b ? _b : "(null)"); \
            fprintf(stderr, "  Got: \"%s\"\n", _a ? _a : "(null)"); \
            return 1; \
        } \
        if (strcmp(_a, _b) != 0) { \
            fprintf(stderr, COLOR_RED COLOR_BOLD "✗ Assertion failed" COLOR_RESET "\n"); \
            fprintf(stderr, "  Location: %s:%d\n", __FILE__, __LINE__); \
            fprintf(stderr, "  Expected: \"%s\"\n", _b); \
            fprintf(stderr, "  Got: \"%s\"\n", _a); \
            return 1; \
        } \
    } while(0)

#define ASSERT_FLOAT_EQ(a, b, epsilon) \
    ASSERT_MSG(fabs((a) - (b)) < (epsilon), \
               "Expected %f, got %f (epsilon=%f)", (double)(b), (double)(a), (double)(epsilon))

/* Type checking assertions */
#define ASSERT_TYPE(obj, type_check) \
    ASSERT_MSG(type_check(obj), "Object has unexpected type")

/* Test statistics tracking */
typedef struct {
    int total;
    int passed;
    int failed;
    double total_time;
} TestStats;

static TestStats g_stats = {0, 0, 0, 0.0};

/* ============================================================================
 * Helper Functions
 * ========================================================================== */

/**
 * Start timing a test
 */
static clock_t test_timer_start(void) {
    return clock();
}

/**
 * End timing and record duration
 */
static double test_timer_end(clock_t start) {
    clock_t end = clock();
    return (double)(end - start) / CLOCKS_PER_SEC;
}

/**
 * Print a visual separator
 */
static void print_separator(void) {
    printf(COLOR_BLUE "-----------------------------------------------------------" COLOR_RESET "\n");
}

/**
 * Print object tree for debugging (with color)
 */
static void debug_print_object(toonObject *obj, int depth) {
    if (!obj) return;
    
    for (int i = 0; i < depth; i++) printf("  ");
    
    if (obj->key) printf(COLOR_YELLOW "%s: " COLOR_RESET, obj->key);
    
    switch (obj->kvtype) {
        case KV_STRING:
            printf(COLOR_GREEN "\"%s\"" COLOR_RESET " (string)\n", obj->str.ptr);
            break;
        case KV_INT:
            printf(COLOR_MAGENTA "%d" COLOR_RESET " (int)\n", obj->i);
            break;
        case KV_DOUBLE:
            printf(COLOR_MAGENTA "%f" COLOR_RESET " (double)\n", obj->d);
            break;
        case KV_BOOL:
            printf(COLOR_CYAN "%s" COLOR_RESET " (bool)\n", obj->boolean ? "true" : "false");
            break;
        case KV_NULL:
            printf(COLOR_RED "null" COLOR_RESET "\n");
            break;
        case KV_LIST:
            printf("[...] (array, len=%zu)\n", obj->array.len);
            break;
        case KV_OBJ:
            printf("{ ... } (object)\n");
            if (obj->child) debug_print_object(obj->child, depth + 1);
            break;
    }
    
    if (obj->next) debug_print_object(obj->next, depth);
}

/* ============================================================================
 * Test Suite Implementation
 * ========================================================================== */

/**
 * Test 1: Basic Parsing - Primitives and Simple Values
 * 
 * Tests fundamental parsing of:
 * - Strings (quoted and unquoted)
 * - Integers
 * - Floating point numbers
 * - Booleans
 * - Null values
 */
static int test_basic_parsing(void) {
    TEST_BEGIN("Basic parsing - primitives and simple values");
    clock_t start = test_timer_start();
    
    const char *toon = 
        "name: John Doe\n"
        "age: 30\n"
        "height: 1.75\n"
        "active: true\n"
        "inactive: false\n"
        "nickname: \"Johnny\"\n"
        "middle_name: null\n"
        "empty_string: \"\"\n"
        "negative_int: -42\n"
        "negative_float: -3.14\n"
        "scientific: 1.5e10\n"
        "scientific_neg: -2.5e-3\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT_NOT_NULL(root);
    
    /* Test string value (unquoted) */
    toonObject *name = TOONc_get(root, "name");
    ASSERT_NOT_NULL(name);
    ASSERT_TYPE(name, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(name), "John Doe");
    
    /* Test integer value */
    toonObject *age = TOONc_get(root, "age");
    ASSERT_NOT_NULL(age);
    ASSERT_TYPE(age, TOON_IS_INT);
    ASSERT_EQ(TOON_GET_INT(age), 30);
    
    /* Test double value */
    toonObject *height = TOONc_get(root, "height");
    ASSERT_NOT_NULL(height);
    ASSERT_TYPE(height, TOON_IS_DOUBLE);
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(height), 1.75, 0.0001);
    
    /* Test boolean true */
    toonObject *active = TOONc_get(root, "active");
    ASSERT_NOT_NULL(active);
    ASSERT_TYPE(active, TOON_IS_BOOL);
    ASSERT_EQ(TOON_GET_BOOL(active), 1);
    
    /* Test boolean false */
    toonObject *inactive = TOONc_get(root, "inactive");
    ASSERT_NOT_NULL(inactive);
    ASSERT_TYPE(inactive, TOON_IS_BOOL);
    ASSERT_EQ(TOON_GET_BOOL(inactive), 0);
    
    /* Test quoted string */
    toonObject *nickname = TOONc_get(root, "nickname");
    ASSERT_NOT_NULL(nickname);
    ASSERT_TYPE(nickname, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(nickname), "Johnny");
    
    /* Test null value */
    toonObject *middle_name = TOONc_get(root, "middle_name");
    ASSERT_NOT_NULL(middle_name);
    ASSERT_TYPE(middle_name, TOON_IS_NULL);
    
    /* Test empty string */
    toonObject *empty = TOONc_get(root, "empty_string");
    ASSERT_NOT_NULL(empty);
    ASSERT_TYPE(empty, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(empty), "");
    
    /* Test negative numbers */
    toonObject *neg_int = TOONc_get(root, "negative_int");
    ASSERT_NOT_NULL(neg_int);
    ASSERT_TYPE(neg_int, TOON_IS_INT);
    ASSERT_EQ(TOON_GET_INT(neg_int), -42);
    
    toonObject *neg_float = TOONc_get(root, "negative_float");
    ASSERT_NOT_NULL(neg_float);
    ASSERT_TYPE(neg_float, TOON_IS_DOUBLE);
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(neg_float), -3.14, 0.0001);
    
    /* Test scientific notation */
    toonObject *scientific = TOONc_get(root, "scientific");
    ASSERT_NOT_NULL(scientific);
    ASSERT_TYPE(scientific, TOON_IS_DOUBLE);
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(scientific), 1.5e10, 1e6);
    
    TOONc_free(root);
    
    double elapsed = test_timer_end(start);
    printf("  Completed in %.3f ms\n", elapsed * 1000);
    TEST_END("Basic parsing");
    return 0;
}

/**
 * Test 2: Nested Objects with Indentation
 * 
 * Tests hierarchical structure parsing with proper nesting.
 * This is critical for validating the stack management fix.
 */
static int test_nested_objects(void) {
    TEST_BEGIN("Nested objects with indentation");
    clock_t start = test_timer_start();
    
    const char *toon = 
        "user:\n"
        "  name: Alice\n"
        "  age: 25\n"
        "  address:\n"
        "    street: 123 Main St\n"
        "    city: Springfield\n"
        "    coordinates:\n"
        "      lat: 42.1234\n"
        "      lon: -71.5678\n"
        "  preferences:\n"
        "    theme: dark\n"
        "    notifications: true\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT_NOT_NULL(root);
    
    /* Verify structure with debug print */
    printf("  Parsed structure:\n");
    debug_print_object(root->child, 1);
    
    /* Test top-level object */
    toonObject *user = TOONc_get(root, "user");
    ASSERT_NOT_NULL(user);
    ASSERT_TYPE(user, TOON_IS_OBJ);
    
    /* Test direct children of user */
    toonObject *name = TOONc_get(root, "user.name");
    ASSERT_MSG(name != NULL, "user.name should not be NULL");
    ASSERT_TYPE(name, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(name), "Alice");
    
    toonObject *age = TOONc_get(root, "user.age");
    ASSERT_NOT_NULL(age);
    ASSERT_TYPE(age, TOON_IS_INT);
    ASSERT_EQ(TOON_GET_INT(age), 25);
    
    /* Test nested address object */
    toonObject *address = TOONc_get(root, "user.address");
    ASSERT_NOT_NULL(address);
    ASSERT_TYPE(address, TOON_IS_OBJ);
    
    toonObject *street = TOONc_get(root, "user.address.street");
    ASSERT_NOT_NULL(street);
    ASSERT_TYPE(street, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(street), "123 Main St");
    
    toonObject *city = TOONc_get(root, "user.address.city");
    ASSERT_NOT_NULL(city);
    ASSERT_TYPE(city, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(city), "Springfield");
    
    /* Test deeply nested coordinates */
    toonObject *lat = TOONc_get(root, "user.address.coordinates.lat");
    ASSERT_NOT_NULL(lat);
    ASSERT_TYPE(lat, TOON_IS_DOUBLE);
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(lat), 42.1234, 0.0001);
    
    toonObject *lon = TOONc_get(root, "user.address.coordinates.lon");
    ASSERT_NOT_NULL(lon);
    ASSERT_TYPE(lon, TOON_IS_DOUBLE);
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(lon), -71.5678, 0.0001);
    
    /* Test sibling object (preferences) */
    toonObject *preferences = TOONc_get(root, "user.preferences");
    ASSERT_NOT_NULL(preferences);
    ASSERT_TYPE(preferences, TOON_IS_OBJ);
    
    toonObject *theme = TOONc_get(root, "user.preferences.theme");
    ASSERT_NOT_NULL(theme);
    ASSERT_TYPE(theme, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(theme), "dark");
    
    toonObject *notifications = TOONc_get(root, "user.preferences.notifications");
    ASSERT_NOT_NULL(notifications);
    ASSERT_TYPE(notifications, TOON_IS_BOOL);
    ASSERT_EQ(TOON_GET_BOOL(notifications), 1);
    
    TOONc_free(root);
    
    double elapsed = test_timer_end(start);
    printf("  Completed in %.3f ms\n", elapsed * 1000);
    TEST_END("Nested objects");
    return 0;
}

/**
 * Test 3: Simple Arrays
 * 
 * Tests array notation [N]: value1,value2,...
 */
static int test_simple_arrays(void) {
    TEST_BEGIN("Simple arrays");
    clock_t start = test_timer_start();
    
    const char *toon = 
        "numbers[5]: 1,2,3,4,5\n"
        "names[3]: alice,bob,charlie\n"
        "mixed[4]: 42,\"hello\",true,null\n"
        "empty[0]:\n"
        "floats[3]: 1.1,2.2,3.3\n"
        "single[1]: only_one\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT_NOT_NULL(root);
    
    /* Test integer array */
    toonObject *numbers = TOONc_get(root, "numbers");
    ASSERT_NOT_NULL(numbers);
    ASSERT_TYPE(numbers, TOON_IS_LIST);
    ASSERT_EQ(TOONc_getArrayLength(numbers), 5);
    
    for (int i = 0; i < 5; i++) {
        toonObject *item = TOONc_getArrayItem(numbers, i);
        ASSERT_NOT_NULL(item);
        ASSERT_TYPE(item, TOON_IS_INT);
        ASSERT_EQ(TOON_GET_INT(item), i + 1);
    }
    
    /* Test string array */
    toonObject *names = TOONc_get(root, "names");
    ASSERT_NOT_NULL(names);
    ASSERT_TYPE(names, TOON_IS_LIST);
    ASSERT_EQ(TOONc_getArrayLength(names), 3);
    
    const char *expected_names[] = {"alice", "bob", "charlie"};
    for (int i = 0; i < 3; i++) {
        toonObject *item = TOONc_getArrayItem(names, i);
        ASSERT_NOT_NULL(item);
        ASSERT_TYPE(item, TOON_IS_STRING);
        ASSERT_STR_EQ(TOON_GET_STRING(item), expected_names[i]);
    }
    
    /* Test mixed type array */
    toonObject *mixed = TOONc_get(root, "mixed");
    ASSERT_NOT_NULL(mixed);
    ASSERT_TYPE(mixed, TOON_IS_LIST);
    ASSERT_EQ(TOONc_getArrayLength(mixed), 4);
    
    ASSERT_TYPE(TOONc_getArrayItem(mixed, 0), TOON_IS_INT);
    ASSERT_EQ(TOON_GET_INT(TOONc_getArrayItem(mixed, 0)), 42);
    
    ASSERT_TYPE(TOONc_getArrayItem(mixed, 1), TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(TOONc_getArrayItem(mixed, 1)), "hello");
    
    ASSERT_TYPE(TOONc_getArrayItem(mixed, 2), TOON_IS_BOOL);
    ASSERT_EQ(TOON_GET_BOOL(TOONc_getArrayItem(mixed, 2)), 1);
    
    ASSERT_TYPE(TOONc_getArrayItem(mixed, 3), TOON_IS_NULL);
    
    /* Test empty array */
    toonObject *empty = TOONc_get(root, "empty");
    ASSERT_NOT_NULL(empty);
    ASSERT_TYPE(empty, TOON_IS_LIST);
    ASSERT_EQ(TOONc_getArrayLength(empty), 0);
    
    /* Test float array */
    toonObject *floats = TOONc_get(root, "floats");
    ASSERT_NOT_NULL(floats);
    ASSERT_TYPE(floats, TOON_IS_LIST);
    ASSERT_EQ(TOONc_getArrayLength(floats), 3);
    
    double expected_floats[] = {1.1, 2.2, 3.3};
    for (int i = 0; i < 3; i++) {
        toonObject *item = TOONc_getArrayItem(floats, i);
        ASSERT_NOT_NULL(item);
        ASSERT_TYPE(item, TOON_IS_DOUBLE);
        ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(item), expected_floats[i], 0.0001);
    }
    
    TOONc_free(root);
    
    double elapsed = test_timer_end(start);
    printf("  Completed in %.3f ms\n", elapsed * 1000);
    TEST_END("Simple arrays");
    return 0;
}

/**
 * Test 4: Tabular Data (CSV-style)
 * 
 * Tests the powerful [N]{col1,col2,...}: notation for structured data.
 */
static int test_tabular_data(void) {
    TEST_BEGIN("Tabular data (CSV-style arrays of objects)");
    clock_t start = test_timer_start();
    
    const char *toon = 
        "users[3]{id,name,email,active}:\n"
        "  1,Alice,alice@example.com,true\n"
        "  2,Bob,bob@example.com,false\n"
        "  3,Charlie,charlie@example.com,true\n"
        "\n"
        "products[2]{id,name,price,category}:\n"
        "  101,Laptop,999.99,Electronics\n"
        "  102,Coffee Mug,15.50,Home\n"
        "\n"
        "empty_table[0]{col1,col2}:\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT_NOT_NULL(root);
    
    /* Test users table */
    toonObject *users = TOONc_get(root, "users");
    ASSERT_NOT_NULL(users);
    ASSERT_TYPE(users, TOON_IS_LIST);
    ASSERT_EQ(TOONc_getArrayLength(users), 3);
    
    /* Verify first user row */
    toonObject *user1 = TOONc_getArrayItem(users, 0);
    ASSERT_NOT_NULL(user1);
    ASSERT_TYPE(user1, TOON_IS_OBJ);
    
    toonObject *u1_id = TOONc_get(user1, "id");
    ASSERT_NOT_NULL(u1_id);
    ASSERT_TYPE(u1_id, TOON_IS_INT);
    ASSERT_EQ(TOON_GET_INT(u1_id), 1);
    
    toonObject *u1_name = TOONc_get(user1, "name");
    ASSERT_NOT_NULL(u1_name);
    ASSERT_TYPE(u1_name, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(u1_name), "Alice");
    
    toonObject *u1_email = TOONc_get(user1, "email");
    ASSERT_NOT_NULL(u1_email);
    ASSERT_TYPE(u1_email, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(u1_email), "alice@example.com");
    
    toonObject *u1_active = TOONc_get(user1, "active");
    ASSERT_NOT_NULL(u1_active);
    ASSERT_TYPE(u1_active, TOON_IS_BOOL);
    ASSERT_EQ(TOON_GET_BOOL(u1_active), 1);
    
    /* Verify second user (different bool value) */
    toonObject *user2 = TOONc_getArrayItem(users, 1);
    ASSERT_NOT_NULL(user2);
    
    toonObject *u2_active = TOONc_get(user2, "active");
    ASSERT_NOT_NULL(u2_active);
    ASSERT_TYPE(u2_active, TOON_IS_BOOL);
    ASSERT_EQ(TOON_GET_BOOL(u2_active), 0);
    
    /* Test products table */
    toonObject *products = TOONc_get(root, "products");
    ASSERT_NOT_NULL(products);
    ASSERT_TYPE(products, TOON_IS_LIST);
    ASSERT_EQ(TOONc_getArrayLength(products), 2);
    
    toonObject *product2 = TOONc_getArrayItem(products, 1);
    ASSERT_NOT_NULL(product2);
    
    toonObject *p2_price = TOONc_get(product2, "price");
    ASSERT_NOT_NULL(p2_price);
    ASSERT_TYPE(p2_price, TOON_IS_DOUBLE);
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(p2_price), 15.50, 0.0001);
    
    /* Test empty table */
    toonObject *empty_table = TOONc_get(root, "empty_table");
    ASSERT_NOT_NULL(empty_table);
    ASSERT_TYPE(empty_table, TOON_IS_LIST);
    ASSERT_EQ(TOONc_getArrayLength(empty_table), 0);
    
    TOONc_free(root);
    
    double elapsed = test_timer_end(start);
    printf("  Completed in %.3f ms\n", elapsed * 1000);
    TEST_END("Tabular data");
    return 0;
}

/**
 * Test 5: Comments and Whitespace
 * 
 * Tests that comments are properly ignored and whitespace is handled correctly.
 */
static int test_comments_and_whitespace(void) {
    TEST_BEGIN("Comments and whitespace handling");
    clock_t start = test_timer_start();
    
    const char *toon = 
        "# This is a header comment\n"
        "  # Indented comment\n"
        "\n"
        "key1: value1\n"
        "# key2: should_be_ignored\n"
        "\n"
        "# Multiple comments\n"
        "# Between values\n"
        "key2: value2\n"
        "\n"
        "  # Comment before nested block\n"
        "parent:\n"
        "  # Comment inside nested block\n"
        "  child: value\n"
        "  # Trailing comment\n"
        "\n"
        "# Final comment\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT_NOT_NULL(root);
    
    /* Verify comments are ignored */
    toonObject *key1 = TOONc_get(root, "key1");
    ASSERT_NOT_NULL(key1);
    ASSERT_TYPE(key1, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(key1), "value1");
    
    toonObject *key2 = TOONc_get(root, "key2");
    ASSERT_NOT_NULL(key2);
    ASSERT_TYPE(key2, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(key2), "value2");
    
    /* Verify nested value works after comments */
    toonObject *child = TOONc_get(root, "parent.child");
    ASSERT_NOT_NULL(child);
    ASSERT_TYPE(child, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(child), "value");
    
    /* Verify that commented key was not parsed */
    toonObject *should_be_ignored = TOONc_get(root, "should_be_ignored");
    ASSERT_NULL(should_be_ignored);
    
    TOONc_free(root);
    
    double elapsed = test_timer_end(start);
    printf("  Completed in %.3f ms\n", elapsed * 1000);
    TEST_END("Comments and whitespace");
    return 0;
}

/**
 * Test 6: Edge Cases and Error Handling
 * 
 * Tests parser robustness with malformed input and boundary conditions.
 */
static int test_edge_cases(void) {
    TEST_BEGIN("Edge cases and error handling");
    clock_t start = test_timer_start();
    
    /* Test 6.1: Empty input */
    printf("  Subtest: Empty input\n");
    toonObject *root1 = TOONc_parseString("");
    ASSERT_NOT_NULL(root1);
    ASSERT_NULL(root1->child);
    TOONc_free(root1);
    
    /* Test 6.2: Only whitespace */
    printf("  Subtest: Whitespace only\n");
    toonObject *root2 = TOONc_parseString("   \n  \t  \n");
    ASSERT_NOT_NULL(root2);
    ASSERT_NULL(root2->child);
    TOONc_free(root2);
    
    /* Test 6.3: Only comments */
    printf("  Subtest: Comments only\n");
    toonObject *root3 = TOONc_parseString("# comment1\n# comment2\n");
    ASSERT_NOT_NULL(root3);
    ASSERT_NULL(root3->child);
    TOONc_free(root3);
    
    /* Test 6.4: Malformed keys (missing colon) */
    printf("  Subtest: Malformed keys\n");
    const char *malformed = 
        "valid: ok\n"
        "no_colon\n"
        "another: valid\n";
    
    toonObject *root4 = TOONc_parseString(malformed);
    ASSERT_NOT_NULL(root4);
    
    toonObject *valid = TOONc_get(root4, "valid");
    ASSERT_NOT_NULL(valid);
    ASSERT_STR_EQ(TOON_GET_STRING(valid), "ok");
    
    toonObject *another = TOONc_get(root4, "another");
    ASSERT_NOT_NULL(another);
    ASSERT_STR_EQ(TOON_GET_STRING(another), "valid");
    
    /* Verify malformed line was skipped */
    toonObject *no_colon = TOONc_get(root4, "no_colon");
    ASSERT_NULL(no_colon);
    
    TOONc_free(root4);
    
    /* Test 6.5: Array bounds checking */
    printf("  Subtest: Array index out of bounds\n");
    const char *array_test = "numbers[2]: 1,2\n";
    toonObject *root5 = TOONc_parseString(array_test);
    ASSERT_NOT_NULL(root5);
    
    toonObject *numbers = TOONc_get(root5, "numbers");
    ASSERT_NOT_NULL(numbers);
    ASSERT_TYPE(numbers, TOON_IS_LIST);
    
    /* Valid indices */
    ASSERT_NOT_NULL(TOONc_getArrayItem(numbers, 0));
    ASSERT_NOT_NULL(TOONc_getArrayItem(numbers, 1));
    
    /* Out of bounds (should return NULL) */
    ASSERT_NULL(TOONc_getArrayItem(numbers, 2));
    ASSERT_NULL(TOONc_getArrayItem(numbers, 100));
    
    TOONc_free(root5);
    
    /* Test 6.6: Leading/trailing whitespace in keys */
    printf("  Subtest: Whitespace in keys\n");
    const char *whitespace_keys = 
        "  key_with_leading_space: value1\n"
        "key_with_trailing_space  : value2\n"
        "  key_with_both  : value3\n";
    
    toonObject *root6 = TOONc_parseString(whitespace_keys);
    ASSERT_NOT_NULL(root6);
    
    /* Keys should be trimmed */
    toonObject *k1 = TOONc_get(root6, "key_with_leading_space");
    ASSERT_NOT_NULL(k1);
    
    toonObject *k2 = TOONc_get(root6, "key_with_trailing_space");
    ASSERT_NOT_NULL(k2);
    
    toonObject *k3 = TOONc_get(root6, "key_with_both");
    ASSERT_NOT_NULL(k3);
    
    TOONc_free(root6);
    
    /* Test 6.7: Very long values */
    printf("  Subtest: Long values\n");
    char long_string[300];
    memset(long_string, 'a', 250);
    long_string[250] = '\0';
    
    char toon_buf[400];
    snprintf(toon_buf, sizeof(toon_buf), "long_key: %s\n", long_string);
    
    toonObject *root7 = TOONc_parseString(toon_buf);
    ASSERT_NOT_NULL(root7);
    
    toonObject *long_val = TOONc_get(root7, "long_key");
    ASSERT_NOT_NULL(long_val);
    ASSERT_TYPE(long_val, TOON_IS_STRING);
    const char *long_str = TOON_GET_STRING(long_val);
    ASSERT_NOT_NULL(long_str);
    ASSERT_EQ(strlen(long_str), 250);
    
    TOONc_free(root7);
    
    double elapsed = test_timer_end(start);
    printf("  Completed in %.3f ms\n", elapsed * 1000);
    TEST_END("Edge cases");
    return 0;
}

/**
 * Test 7: Object Creation API
 * 
 * Tests programmatic object creation and manipulation.
 */
static int test_object_creation(void) {
    TEST_BEGIN("Object creation API");
    clock_t start = test_timer_start();
    
    /* Create root object */
    toonObject *root = TOONc_newObject(KV_OBJ);
    ASSERT_NOT_NULL(root);
    
    /* Create various typed properties */
    toonObject *name = TOONc_newStringObj("Test User", 9);
    name->key = strdup("name");
    
    toonObject *age = TOONc_newIntObj(42);
    age->key = strdup("age");
    
    toonObject *score = TOONc_newDoubleObj(95.5);
    score->key = strdup("score");
    
    toonObject *active = TOONc_newBoolObj(1);
    active->key = strdup("active");
    
    toonObject *null_val = TOONc_newNullObj();
    null_val->key = strdup("null_field");
    
    /* Create array */
    toonObject *tags = TOONc_newListObj();
    tags->key = strdup("tags");
    TOONc_listPush(tags, TOONc_newStringObj("admin", 5));
    TOONc_listPush(tags, TOONc_newStringObj("user", 4));
    TOONc_listPush(tags, TOONc_newStringObj("tester", 6));
    
    /* Link properties as siblings */
    root->child = name;
    name->next = age;
    age->next = score;
    score->next = active;
    active->next = null_val;
    null_val->next = tags;
    
    /* Verify all properties */
    ASSERT_TYPE(name, TOON_IS_STRING);
    ASSERT_STR_EQ(TOON_GET_STRING(name), "Test User");
    
    ASSERT_TYPE(age, TOON_IS_INT);
    ASSERT_EQ(TOON_GET_INT(age), 42);
    
    ASSERT_TYPE(score, TOON_IS_DOUBLE);
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(score), 95.5, 0.0001);
    
    ASSERT_TYPE(active, TOON_IS_BOOL);
    ASSERT_EQ(TOON_GET_BOOL(active), 1);
    
    ASSERT_TYPE(null_val, TOON_IS_NULL);
    
    ASSERT_TYPE(tags, TOON_IS_LIST);
    ASSERT_EQ(TOONc_getArrayLength(tags), 3);
    
    /* Test TOONc_get on created structure */
    toonObject *found = TOONc_get(root, "name");
    ASSERT_NOT_NULL(found);
    ASSERT(found == name);
    
    /* Print for visual verification */
    printf("  Programmatically created object:\n");
    debug_print_object(root->child, 2);
    
    /* Test JSON export */
    printf("  JSON representation:\n");
    FILE *json_out = tmpfile();
    ASSERT_NOT_NULL(json_out);
    TOONc_toJSON(root, json_out, 0);
    rewind(json_out);
    
    char json_buf[1024];
    size_t read = fread(json_buf, 1, sizeof(json_buf) - 1, json_out);
    json_buf[read] = '\0';
    printf("    %s\n", json_buf);
    fclose(json_out);
    
    TOONc_free(root);
    
    double elapsed = test_timer_end(start);
    printf("  Completed in %.3f ms\n", elapsed * 1000);
    TEST_END("Object creation");
    return 0;
}

/**
 * Test 8: Memory Management
 * 
 * Verifies memory allocation functions and proper cleanup.
 */
static int test_memory_management(void) {
    TEST_BEGIN("Memory management");
    clock_t start = test_timer_start();
    
    /* Test allocation wrappers */
    printf("  Testing allocation functions...\n");
    
    void *ptr1 = TOONc_malloc(100);
    ASSERT_NOT_NULL(ptr1);
    memset(ptr1, 0xFF, 100);
    
    void *ptr2 = TOONc_calloc(10, sizeof(int));
    ASSERT_NOT_NULL(ptr2);
    
    /* Verify calloc zeros memory */
    int *int_ptr = (int *)ptr2;
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(int_ptr[i], 0);
    }
    
    /* Test realloc */
    void *ptr3 = TOONc_realloc(ptr1, 200);
    ASSERT_NOT_NULL(ptr3);
    
    free(ptr2);
    free(ptr3);
    
    /* Test deep object tree freeing */
    printf("  Testing deep tree cleanup...\n");
    const char *complex = 
        "level1:\n"
        "  level2:\n"
        "    level3:\n"
        "      level4:\n"
        "        deep_value: reached\n"
        "  array[3]: a,b,c\n"
        "  table[2]{x,y}:\n"
        "    1,2\n"
        "    3,4\n";
    
    toonObject *root = TOONc_parseString(complex);
    ASSERT_NOT_NULL(root);
    
    /* Verify structure before freeing */
    toonObject *deep = TOONc_get(root, "level1.level2.level3.level4.deep_value");
    ASSERT_NOT_NULL(deep);
    
    /* This should not leak - verify with valgrind */
    TOONc_free(root);
    
    double elapsed = test_timer_end(start);
    printf("  Completed in %.3f ms\n", elapsed * 1000);
    TEST_END("Memory management");
    return 0;
}

/**
 * Test 9: Type Checking and Getters
 * 
 * Validates type checking macros and safe getter functions.
 */
static int test_type_checking(void) {
    TEST_BEGIN("Type checking and getter macros");
    clock_t start = test_timer_start();
    
    const char *toon = 
        "string: hello\n"
        "integer: 42\n"
        "double: 3.14159\n"
        "bool_true: true\n"
        "bool_false: false\n"
        "null_val: null\n"
        "array[2]: a,b\n"
        "object:\n"
        "  nested: value\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT_NOT_NULL(root);
    
    /* Test positive type checks */
    printf("  Testing type identification...\n");
    
    toonObject *str = TOONc_get(root, "string");
    ASSERT(TOON_IS_STRING(str) && !TOON_IS_INT(str));
    
    toonObject *integer = TOONc_get(root, "integer");
    ASSERT(TOON_IS_INT(integer) && !TOON_IS_STRING(integer));
    
    toonObject *dbl = TOONc_get(root, "double");
    ASSERT(TOON_IS_DOUBLE(dbl) && !TOON_IS_INT(dbl));
    
    toonObject *btrue = TOONc_get(root, "bool_true");
    ASSERT(TOON_IS_BOOL(btrue));
    ASSERT_EQ(TOON_GET_BOOL(btrue), 1);
    
    toonObject *bfalse = TOONc_get(root, "bool_false");
    ASSERT(TOON_IS_BOOL(bfalse));
    ASSERT_EQ(TOON_GET_BOOL(bfalse), 0);
    
    toonObject *null = TOONc_get(root, "null_val");
    ASSERT(TOON_IS_NULL(null));
    
    toonObject *arr = TOONc_get(root, "array");
    ASSERT(TOON_IS_LIST(arr));
    
    toonObject *obj = TOONc_get(root, "object");
    ASSERT(TOON_IS_OBJ(obj));
    
    /* Test safe getters on wrong types (should return defaults) */
    printf("  Testing type-safe getters...\n");
    
    ASSERT_EQ(TOON_GET_INT(str), 0);
    ASSERT_NULL(TOON_GET_STRING(integer));
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(btrue), 0.0, 0.0001);
    ASSERT_EQ(TOON_GET_BOOL(null), 0);
    
    TOONc_free(root);
    
    double elapsed = test_timer_end(start);
    printf("  Completed in %.3f ms\n", elapsed * 1000);
    TEST_END("Type checking");
    return 0;
}

/**
 * Test 10: Complex Real-World Structure
 * 
 * Tests a realistic configuration file with multiple nesting levels,
 * mixed data types, and tabular data.
 */
static int test_complex_structure(void) {
    TEST_BEGIN("Complex real-world structure");
    clock_t start = test_timer_start();
    
    const char *toon = 
        "# Application Configuration\n"
        "app:\n"
        "  name: MyApp\n"
        "  version: 2.1.0\n"
        "  build: 12345\n"
        "  settings:\n"
        "    debug: true\n"
        "    log_level: verbose\n"
        "    max_connections: 100\n"
        "    timeout: 30.5\n"
        "    features[3]: auth,api,ui\n"
        "\n"
        "database:\n"
        "  type: postgresql\n"
        "  hosts[2]: db1.example.com,db2.example.com\n"
        "  config:\n"
        "    pool_size: 10\n"
        "    timeout: 30.5\n"
        "    ssl: true\n"
        "\n"
        "users[3]{id,name,role,active}:\n"
        "  1,Admin,admin,true\n"
        "  2,User,user,true\n"
        "  3,Guest,guest,false\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT_NOT_NULL(root);
    
    printf("  Parsed structure:\n");
    debug_print_object(root->child, 2);
    
    /* Test deep paths */
    printf("  Testing deep path queries...\n");
    
    toonObject *app_name = TOONc_get(root, "app.name");
    ASSERT_MSG(app_name != NULL, "app.name path failed");
    ASSERT_STR_EQ(TOON_GET_STRING(app_name), "MyApp");
    
    toonObject *build = TOONc_get(root, "app.build");
    ASSERT_NOT_NULL(build);
    ASSERT_EQ(TOON_GET_INT(build), 12345);
    
    toonObject *debug = TOONc_get(root, "app.settings.debug");
    ASSERT_NOT_NULL(debug);
    ASSERT_EQ(TOON_GET_BOOL(debug), 1);
    
    toonObject *features = TOONc_get(root, "app.settings.features");
    ASSERT_NOT_NULL(features);
    ASSERT_TYPE(features, TOON_IS_LIST);
    ASSERT_EQ(TOONc_getArrayLength(features), 3);
    
    /* Test database configuration */
    toonObject *db_type = TOONc_get(root, "database.type");
    ASSERT_NOT_NULL(db_type);
    ASSERT_STR_EQ(TOON_GET_STRING(db_type), "postgresql");
    
    toonObject *hosts = TOONc_get(root, "database.hosts");
    ASSERT_NOT_NULL(hosts);
    ASSERT_TYPE(hosts, TOON_IS_LIST);
    ASSERT_EQ(TOONc_getArrayLength(hosts), 2);
    
    toonObject *timeout = TOONc_get(root, "database.config.timeout");
    ASSERT_NOT_NULL(timeout);
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(timeout), 30.5, 0.0001);
    
    /* Test table data */
    printf("  Testing tabular data access...\n");
    
    toonObject *users = TOONc_get(root, "users");
    ASSERT_NOT_NULL(users);
    ASSERT_TYPE(users, TOON_IS_LIST);
    ASSERT_EQ(TOONc_getArrayLength(users), 3);
    
    toonObject *user2 = TOONc_getArrayItem(users, 1);
    ASSERT_NOT_NULL(user2);
    ASSERT_TYPE(user2, TOON_IS_OBJ);
    
    toonObject *u2_name = TOONc_get(user2, "name");
    ASSERT_NOT_NULL(u2_name);
    ASSERT_STR_EQ(TOON_GET_STRING(u2_name), "User");
    
    toonObject *u2_role = TOONc_get(user2, "role");
    ASSERT_NOT_NULL(u2_role);
    ASSERT_STR_EQ(TOON_GET_STRING(u2_role), "user");
    
    /* Export to JSON */
    printf("  JSON export:\n");
    FILE *json = tmpfile();
    TOONc_toJSON(root, json, 0);
    printf("    [JSON output written to temp file]\n");
    fclose(json);
    
    TOONc_free(root);
    
    double elapsed = test_timer_end(start);
    printf("  Completed in %.3f ms\n", elapsed * 1000);
    TEST_END("Complex structure");
    return 0;
}

/**
 * Test 11: File I/O
 * 
 * Tests parsing from actual files.
 */
static int test_file_io(void) {
    TEST_BEGIN("File I/O operations");
    clock_t start = test_timer_start();
    
    const char *content = 
        "test_file: true\n"
        "value: 123\n"
        "nested:\n"
        "  item: file_test\n"
        "  level2:\n"
        "    deep: value\n";
    
    /* Create temporary file */
    FILE *fp = tmpfile();
    ASSERT_NOT_NULL(fp);
    
    size_t written = fwrite(content, 1, strlen(content), fp);
    ASSERT_EQ(written, strlen(content));
    rewind(fp);
    
    /* Parse from file */
    toonObject *root = TOONc_parseFile(fp);
    ASSERT_NOT_NULL(root);
    
    /* Verify content */
    toonObject *test_file = TOONc_get(root, "test_file");
    ASSERT_NOT_NULL(test_file);
    ASSERT_TYPE(test_file, TOON_IS_BOOL);
    ASSERT_EQ(TOON_GET_BOOL(test_file), 1);
    
    toonObject *value = TOONc_get(root, "value");
    ASSERT_NOT_NULL(value);
    ASSERT_EQ(TOON_GET_INT(value), 123);
    
    toonObject *item = TOONc_get(root, "nested.item");
    ASSERT_NOT_NULL(item);
    ASSERT_STR_EQ(TOON_GET_STRING(item), "file_test");
    
    toonObject *deep = TOONc_get(root, "nested.level2.deep");
    ASSERT_NOT_NULL(deep);
    ASSERT_STR_EQ(TOON_GET_STRING(deep), "value");
    
    TOONc_free(root);
    
    double elapsed = test_timer_end(start);
    printf("  Completed in %.3f ms\n", elapsed * 1000);
    TEST_END("File I/O");
    return 0;
}

/**
 * Test 12: Performance Benchmark
 * 
 * Stress test with large documents.
 */
static int test_performance(void) {
    TEST_BEGIN("Performance benchmark");
    
    printf("  Generating large document...\n");
    
    /* Allocate buffer for large document */
    size_t buf_size = 2000000; /* 2MB */
    char *large_toon = malloc(buf_size);
    ASSERT_NOT_NULL(large_toon);
    
    char *ptr = large_toon;
    size_t remaining = buf_size;
    
    /* Header */
    int n = snprintf(ptr, remaining, "large_dataset:\n");
    ptr += n; remaining -= n;
    
    /* Create 1000 nested records */
    for (int i = 0; i < 1000 && remaining > 200; i++) {
        n = snprintf(ptr, remaining, 
            "  record_%d:\n"
            "    id: %d\n"
            "    name: User%d\n"
            "    value: %.2f\n"
            "    active: %s\n",
            i, i, i, i * 1.5, i % 2 == 0 ? "true" : "false");
        ptr += n; remaining -= n;
    }
    
    printf("  Parsing %zu bytes...\n", ptr - large_toon);
    
    /* Parse and time */
    clock_t start = test_timer_start();
    toonObject *root = TOONc_parseString(large_toon);
    double parse_time = test_timer_end(start);
    
    ASSERT_NOT_NULL(root);
    
    printf("  " COLOR_BOLD "Parse time: %.3f ms" COLOR_RESET "\n", parse_time * 1000);
    printf("  " COLOR_BOLD "Throughput: %.2f MB/s" COLOR_RESET "\n", 
           (ptr - large_toon) / (1024.0 * 1024.0) / parse_time);
    
    /* Verify random sample */
    toonObject *record_500 = TOONc_get(root, "large_dataset.record_500");
    ASSERT_NOT_NULL(record_500);
    
    toonObject *id_500 = TOONc_get(record_500, "id");
    ASSERT_NOT_NULL(id_500);
    ASSERT_EQ(TOON_GET_INT(id_500), 500);
    
    /* Cleanup */
    start = clock();
    TOONc_free(root);
    double free_time = test_timer_end(start);
    
    printf("  Free time: %.3f ms\n", free_time * 1000);
    
    free(large_toon);
    
    TEST_END("Performance");
    return 0;
}

/* ============================================================================
 * Main Test Runner
 * ========================================================================== */

typedef int (*test_func_t)(void);

typedef struct {
    const char *name;
    test_func_t func;
    int enabled;
} TestCase;

int main(int argc __attribute__((unused)), char **argv __attribute__((unused))) {
    printf("\n");
    printf(COLOR_BOLD COLOR_CYAN);
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║       TOONc Test Suite - Professional Edition         ║\n");
    printf("║  Token-Oriented Object Notation Parser Library        ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf(COLOR_RESET "\n");
    
    /* Test cases registry */
    TestCase tests[] = {
        {"Basic Parsing", test_basic_parsing, 1},
        {"Nested Objects", test_nested_objects, 1},
        {"Simple Arrays", test_simple_arrays, 1},
        {"Tabular Data", test_tabular_data, 1},
        {"Comments & Whitespace", test_comments_and_whitespace, 1},
        {"Edge Cases", test_edge_cases, 1},
        {"Object Creation API", test_object_creation, 1},
        {"Memory Management", test_memory_management, 1},
        {"Type Checking", test_type_checking, 1},
        {"Complex Structure", test_complex_structure, 1},
        {"File I/O", test_file_io, 1},
        {"Performance", test_performance, 1},
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    
    /* Run all enabled tests */
    clock_t suite_start = clock();
    g_stats.total = 0;
    g_stats.passed = 0;
    g_stats.failed = 0;
    
    for (int i = 0; i < num_tests; i++) {
        if (!tests[i].enabled) continue;
        
        g_stats.total++;
        
        printf(COLOR_BOLD "[%d/%d] " COLOR_RESET, i + 1, num_tests);
        
        int result = tests[i].func();
        
        if (result == 0) {
            g_stats.passed++;
        } else {
            g_stats.failed++;
            printf(COLOR_RED "  ✗ Test FAILED: %s" COLOR_RESET "\n", tests[i].name);
        }
        
        print_separator();
    }
    
    double suite_time = (double)(clock() - suite_start) / CLOCKS_PER_SEC;
    g_stats.total_time = suite_time;
    
    /* Print summary */
    printf("\n");
    printf(COLOR_BOLD COLOR_CYAN);
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║                    Test Summary                        ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf(COLOR_RESET);
    
    printf("  Total tests:   %d\n", g_stats.total);
    printf("  " COLOR_GREEN "Passed:        %d" COLOR_RESET "\n", g_stats.passed);
    
    if (g_stats.failed > 0) {
        printf("  " COLOR_RED COLOR_BOLD "Failed:        %d" COLOR_RESET "\n", g_stats.failed);
    } else {
        printf("  Failed:        0\n");
    }
    
    printf("  Total time:    %.3f seconds\n", suite_time);
    printf("  Avg per test:  %.3f ms\n", (suite_time * 1000) / g_stats.total);
    
    printf("\n");
    
    if (g_stats.failed == 0) {
        printf(COLOR_GREEN COLOR_BOLD);
        printf("  ╔══════════════════════════════════════════╗\n");
        printf("  ║   ✓ ALL TESTS PASSED SUCCESSFULLY! ✓    ║\n");
        printf("  ╚══════════════════════════════════════════╝\n");
        printf(COLOR_RESET "\n");
        return EXIT_SUCCESS;
    } else {
        printf(COLOR_RED COLOR_BOLD);
        printf("  ╔══════════════════════════════════════════╗\n");
        printf("  ║   ✗ SOME TESTS FAILED - SEE ABOVE ✗     ║\n");
        printf("  ╚══════════════════════════════════════════╝\n");
        printf(COLOR_RESET "\n");
        return EXIT_FAILURE;
    }
}
