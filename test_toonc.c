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
 * Test suite for TOONc - Token-Oriented Object Notation parser library
 * Tests all major functionality including parsing, querying, and object creation
 */

#include "toonc.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Test helper macros */
#define TEST(name) printf("\n=== Testing: %s ===\n", name)
#define ASSERT(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "Assertion failed at %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        return 1; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_STR_EQ(a, b) ASSERT(strcmp((a), (b)) == 0)
#define ASSERT_FLOAT_EQ(a, b, eps) ASSERT(fabs((a) - (b)) < (eps))

/* -----------------------------------------------------------------------------
 * Test 1: Basic parsing - simple key-value pairs
 * -------------------------------------------------------------------------- */
int test_basic_parsing() {
    TEST("Basic parsing - simple key-value pairs");
    
    const char *toon = 
        "name: John Doe\n"
        "age: 30\n"
        "height: 1.75\n"
        "active: true\n"
        "nickname: \"Johnny\"\n"
        "middle_name: null\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT(root != NULL);
    
    /* Test string value */
    toonObject *name = TOONc_get(root, "name");
    ASSERT(name != NULL);
    ASSERT(TOON_IS_STRING(name));
    ASSERT_STR_EQ(TOON_GET_STRING(name), "John Doe");
    
    /* Test integer value */
    toonObject *age = TOONc_get(root, "age");
    ASSERT(age != NULL);
    ASSERT(TOON_IS_INT(age));
    ASSERT_EQ(TOON_GET_INT(age), 30);
    
    /* Test double value */
    toonObject *height = TOONc_get(root, "height");
    ASSERT(height != NULL);
    ASSERT(TOON_IS_DOUBLE(height));
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(height), 1.75, 0.0001);
    
    /* Test boolean value */
    toonObject *active = TOONc_get(root, "active");
    ASSERT(active != NULL);
    ASSERT(TOON_IS_BOOL(active));
    ASSERT_EQ(TOON_GET_BOOL(active), 1);
    
    /* Test quoted string */
    toonObject *nickname = TOONc_get(root, "nickname");
    ASSERT(nickname != NULL);
    ASSERT(TOON_IS_STRING(nickname));
    ASSERT_STR_EQ(TOON_GET_STRING(nickname), "Johnny");
    
    /* Test null value */
    toonObject *middle_name = TOONc_get(root, "middle_name");
    ASSERT(middle_name != NULL);
    ASSERT(TOON_IS_NULL(middle_name));
    
    TOONc_free(root);
    printf("✓ Basic parsing passed\n");
    return 0;
}

/* -----------------------------------------------------------------------------
 * Test 2: Nested objects with indentation
 * -------------------------------------------------------------------------- */
int test_nested_objects() {
    TEST("Nested objects with indentation");
    
    const char *toon = 
        "user:\n"
        "  name: Alice\n"
        "  age: 25\n"
        "  address:\n"
        "    street: 123 Main St\n"
        "    city: Springfield\n"
        "  preferences:\n"
        "    theme: dark\n"
        "    notifications: true\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT(root != NULL);
    
    /* Test top-level object */
    toonObject *user = TOONc_get(root, "user");
    ASSERT(user != NULL);
    ASSERT(TOON_IS_OBJ(user));
    
    /* Test nested properties */
    toonObject *name = TOONc_get(root, "user.name");
    printf("%p\n", name);
    ASSERT(name != NULL);
    ASSERT(TOON_IS_STRING(name));
    ASSERT_STR_EQ(TOON_GET_STRING(name), "Alice");
    
    toonObject *age = TOONc_get(root, "user.age");
    ASSERT(age != NULL);
    ASSERT(TOON_IS_INT(age));
    ASSERT_EQ(TOON_GET_INT(age), 25);
    
    /* Test deeper nesting */
    toonObject *street = TOONc_get(root, "user.address.street");
    ASSERT(street != NULL);
    ASSERT(TOON_IS_STRING(street));
    ASSERT_STR_EQ(TOON_GET_STRING(street), "123 Main St");
    
    toonObject *city = TOONc_get(root, "user.address.city");
    ASSERT(city != NULL);
    ASSERT(TOON_IS_STRING(city));
    ASSERT_STR_EQ(TOON_GET_STRING(city), "Springfield");
    
    /* Test boolean in nested object */
    toonObject *notifications = TOONc_get(root, "user.preferences.notifications");
    ASSERT(notifications != NULL);
    ASSERT(TOON_IS_BOOL(notifications));
    ASSERT_EQ(TOON_GET_BOOL(notifications), 1);
    
    TOONc_free(root);
    printf("✓ Nested objects passed\n");
    return 0;
}

/* -----------------------------------------------------------------------------
 * Test 3: Simple arrays
 * -------------------------------------------------------------------------- */
int test_simple_arrays() {
    TEST("Simple arrays");
    
    const char *toon = 
        "numbers[5]: 1,2,3,4,5\n"
        "names[3]: alice,bob,charlie\n"
        "mixed[4]: 42,\"hello\",true,null\n"
        "empty[0]:\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT(root != NULL);
    
    /* Test integer array */
    toonObject *numbers = TOONc_get(root, "numbers");
    ASSERT(numbers != NULL);
    ASSERT(TOON_IS_LIST(numbers));
    ASSERT_EQ(TOONc_getArrayLength(numbers), 5);
    
    ASSERT_EQ(TOON_GET_INT(TOONc_getArrayItem(numbers, 0)), 1);
    ASSERT_EQ(TOON_GET_INT(TOONc_getArrayItem(numbers, 1)), 2);
    ASSERT_EQ(TOON_GET_INT(TOONc_getArrayItem(numbers, 2)), 3);
    ASSERT_EQ(TOON_GET_INT(TOONc_getArrayItem(numbers, 3)), 4);
    ASSERT_EQ(TOON_GET_INT(TOONc_getArrayItem(numbers, 4)), 5);
    
    /* Test string array */
    toonObject *names = TOONc_get(root, "names");
    ASSERT(names != NULL);
    ASSERT(TOON_IS_LIST(names));
    ASSERT_EQ(TOONc_getArrayLength(names), 3);
    
    ASSERT_STR_EQ(TOON_GET_STRING(TOONc_getArrayItem(names, 0)), "alice");
    ASSERT_STR_EQ(TOON_GET_STRING(TOONc_getArrayItem(names, 1)), "bob");
    ASSERT_STR_EQ(TOON_GET_STRING(TOONc_getArrayItem(names, 2)), "charlie");
    
    /* Test mixed type array */
    toonObject *mixed = TOONc_get(root, "mixed");
    ASSERT(mixed != NULL);
    ASSERT(TOON_IS_LIST(mixed));
    ASSERT_EQ(TOONc_getArrayLength(mixed), 4);
    
    ASSERT(TOON_IS_INT(TOONc_getArrayItem(mixed, 0)));
    ASSERT_EQ(TOON_GET_INT(TOONc_getArrayItem(mixed, 0)), 42);
    
    ASSERT(TOON_IS_STRING(TOONc_getArrayItem(mixed, 1)));
    ASSERT_STR_EQ(TOON_GET_STRING(TOONc_getArrayItem(mixed, 1)), "hello");
    
    ASSERT(TOON_IS_BOOL(TOONc_getArrayItem(mixed, 2)));
    ASSERT_EQ(TOON_GET_BOOL(TOONc_getArrayItem(mixed, 2)), 1);
    
    ASSERT(TOON_IS_NULL(TOONc_getArrayItem(mixed, 3)));
    
    /* Test empty array */
    toonObject *empty = TOONc_get(root, "empty");
    ASSERT(empty != NULL);
    ASSERT(TOON_IS_LIST(empty));
    ASSERT_EQ(TOONc_getArrayLength(empty), 0);
    
    TOONc_free(root);
    printf("✓ Simple arrays passed\n");
    return 0;
}

/* -----------------------------------------------------------------------------
 * Test 4: Tabular data (CSV-style arrays of objects)
 * -------------------------------------------------------------------------- */
int test_tabular_data() {
    TEST("Tabular data (CSV-style arrays of objects)");
    
    const char *toon = 
        "users[3]{id,name,email,active}:\n"
        "  1,Alice,alice@example.com,true\n"
        "  2,Bob,bob@example.com,false\n"
        "  3,Charlie,charlie@example.com,true\n"
        "\n"
        "products[2]{id,name,price,category}:\n"
        "  101,Laptop,999.99,Electronics\n"
        "  102,Coffee Mug,15.50,Home\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT(root != NULL);
    
    /* Test users table */
    toonObject *users = TOONc_get(root, "users");
    ASSERT(users != NULL);
    ASSERT(TOON_IS_LIST(users));
    ASSERT_EQ(TOONc_getArrayLength(users), 3);
    
    /* Test first user */
    toonObject *user1 = TOONc_getArrayItem(users, 0);
    ASSERT(user1 != NULL);
    ASSERT(TOON_IS_OBJ(user1));
    
    toonObject *id1 = TOONc_get(user1, "id");
    ASSERT(id1 != NULL);
    ASSERT(TOON_IS_INT(id1));
    ASSERT_EQ(TOON_GET_INT(id1), 1);
    
    toonObject *name1 = TOONc_get(user1, "name");
    ASSERT(name1 != NULL);
    ASSERT(TOON_IS_STRING(name1));
    ASSERT_STR_EQ(TOON_GET_STRING(name1), "Alice");
    
    toonObject *email1 = TOONc_get(user1, "email");
    ASSERT(email1 != NULL);
    ASSERT(TOON_IS_STRING(email1));
    ASSERT_STR_EQ(TOON_GET_STRING(email1), "alice@example.com");
    
    toonObject *active1 = TOONc_get(user1, "active");
    ASSERT(active1 != NULL);
    ASSERT(TOON_IS_BOOL(active1));
    ASSERT_EQ(TOON_GET_BOOL(active1), 1);
    
    /* Test second user */
    toonObject *user2 = TOONc_getArrayItem(users, 1);
    ASSERT(user2 != NULL);
    
    toonObject *active2 = TOONc_get(user2, "active");
    ASSERT(active2 != NULL);
    ASSERT(TOON_IS_BOOL(active2));
    ASSERT_EQ(TOON_GET_BOOL(active2), 0);
    
    /* Test products table */
    toonObject *products = TOONc_get(root, "products");
    ASSERT(products != NULL);
    ASSERT(TOON_IS_LIST(products));
    ASSERT_EQ(TOONc_getArrayLength(products), 2);
    
    toonObject *product2 = TOONc_getArrayItem(products, 1);
    ASSERT(product2 != NULL);
    
    toonObject *price2 = TOONc_get(product2, "price");
    ASSERT(price2 != NULL);
    ASSERT(TOON_IS_DOUBLE(price2));
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(price2), 15.50, 0.0001);
    
    TOONc_free(root);
    printf("✓ Tabular data passed\n");
    return 0;
}

/* -----------------------------------------------------------------------------
 * Test 5: Comments and whitespace handling
 * -------------------------------------------------------------------------- */
int test_comments_and_whitespace() {
    TEST("Comments and whitespace handling");
    
    const char *toon = 
        "# This is a comment\n"
        "  # Indented comment\n"
        "\n"
        "key1: value1  # Inline comment\n"
        "\n"
        "# Multiple comments\n"
        "# Between values\n"
        "key2: value2\n"
        "\n"
        "  # Comment in indented block\n"
        "parent:\n"
        "  child: value\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT(root != NULL);
    
    /* Test that comments are ignored */
    toonObject *key1 = TOONc_get(root, "key1");
    ASSERT(key1 != NULL);
    ASSERT(TOON_IS_STRING(key1));
    ASSERT_STR_EQ(TOON_GET_STRING(key1), "value1");
    
    toonObject *key2 = TOONc_get(root, "key2");
    ASSERT(key2 != NULL);
    ASSERT(TOON_IS_STRING(key2));
    ASSERT_STR_EQ(TOON_GET_STRING(key2), "value2");
    
    /* Test nested value after comment */
    toonObject *child = TOONc_get(root, "parent.child");
    ASSERT(child != NULL);
    ASSERT(TOON_IS_STRING(child));
    ASSERT_STR_EQ(TOON_GET_STRING(child), "value");
    
    TOONc_free(root);
    printf("✓ Comments and whitespace passed\n");
    return 0;
}

/* -----------------------------------------------------------------------------
 * Test 6: Edge cases and error handling
 * -------------------------------------------------------------------------- */
int test_edge_cases() {
    TEST("Edge cases and error handling");
    
    /* Test empty input */
    toonObject *root1 = TOONc_parseString("");
    ASSERT(root1 != NULL);
    ASSERT(root1->child == NULL);
    TOONc_free(root1);
    
    /* Test only whitespace */
    toonObject *root2 = TOONc_parseString("   \n  \t  \n");
    ASSERT(root2 != NULL);
    ASSERT(root2->child == NULL);
    TOONc_free(root2);
    
    /* Test malformed keys (should skip line) */
    const char *malformed = 
        "valid: ok\n"
        "no_colon\n"
        "another: valid\n";
    
    toonObject *root3 = TOONc_parseString(malformed);
    ASSERT(root3 != NULL);
    
    toonObject *valid = TOONc_get(root3, "valid");
    ASSERT(valid != NULL);
    ASSERT_STR_EQ(TOON_GET_STRING(valid), "ok");
    
    toonObject *another = TOONc_get(root3, "another");
    ASSERT(another != NULL);
    ASSERT_STR_EQ(TOON_GET_STRING(another), "valid");
    
    /* Test that no_colon was skipped */
    toonObject *no_colon = TOONc_get(root3, "no_colon");
    ASSERT(no_colon == NULL);
    
    TOONc_free(root3);
    
    /* Test array index out of bounds */
    const char *array_test = "numbers[2]: 1,2\n";
    toonObject *root4 = TOONc_parseString(array_test);
    ASSERT(root4 != NULL);
    
    toonObject *numbers = TOONc_get(root4, "numbers");
    ASSERT(numbers != NULL);
    
    /* Valid access */
    ASSERT(TOONc_getArrayItem(numbers, 0) != NULL);
    ASSERT(TOONc_getArrayItem(numbers, 1) != NULL);
    
    /* Out of bounds should return NULL */
    ASSERT(TOONc_getArrayItem(numbers, 2) == NULL);
    ASSERT(TOONc_getArrayItem(numbers, -1) == NULL);
    
    TOONc_free(root4);
    
    printf("✓ Edge cases passed\n");
    return 0;
}

/* -----------------------------------------------------------------------------
 * Test 7: Object creation API
 * -------------------------------------------------------------------------- */
int test_object_creation() {
    TEST("Object creation API");
    
    /* Create a complex object programmatically */
    toonObject *root = TOONc_newObject(KV_OBJ);
    
    /* Create properties */
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
    
    /* Create an array */
    toonObject *tags = TOONc_newListObj();
    tags->key = strdup("tags");
    TOONc_listPush(tags, TOONc_newStringObj("admin", 5));
    TOONc_listPush(tags, TOONc_newStringObj("user", 4));
    TOONc_listPush(tags, TOONc_newStringObj("tester", 6));
    
    /* Link properties */
    root->child = name;
    name->next = age;
    age->next = score;
    score->next = active;
    active->next = null_val;
    null_val->next = tags;
    
    /* Verify created object */
    ASSERT(TOON_IS_STRING(name));
    ASSERT_STR_EQ(TOON_GET_STRING(name), "Test User");
    
    ASSERT(TOON_IS_INT(age));
    ASSERT_EQ(TOON_GET_INT(age), 42);
    
    ASSERT(TOON_IS_DOUBLE(score));
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(score), 95.5, 0.0001);
    
    ASSERT(TOON_IS_BOOL(active));
    ASSERT_EQ(TOON_GET_BOOL(active), 1);
    
    ASSERT(TOON_IS_NULL(null_val));
    
    ASSERT(TOON_IS_LIST(tags));
    ASSERT_EQ(TOONc_getArrayLength(tags), 3);
    ASSERT_STR_EQ(TOON_GET_STRING(TOONc_getArrayItem(tags, 0)), "admin");
    
    /* Test TOONc_get on programmatically created object */
    toonObject *found_name = TOONc_get(root, "name");
    ASSERT(found_name != NULL);
    ASSERT(found_name == name);
    
    toonObject *found_tag = TOONc_get(root, "tags");
    ASSERT(found_tag != NULL);
    ASSERT(found_tag == tags);
    
    /* Test array access */
    toonObject *first_tag = TOONc_getArrayItem(tags, 0);
    ASSERT(first_tag != NULL);
    ASSERT_STR_EQ(TOON_GET_STRING(first_tag), "admin");
    
    /* Print for visual verification */
    printf("Programmatically created object:\n");
    TOONc_printObject(root, 0);
    
    /* Convert to JSON */
    printf("\nJSON output:\n");
    TOONc_toJSON(root, stdout, 0);
    printf("\n");
    
    TOONc_free(root);
    printf("✓ Object creation passed\n");
    return 0;
}

/* -----------------------------------------------------------------------------
 * Test 8: Memory management
 * -------------------------------------------------------------------------- */
int test_memory_management() {
    TEST("Memory management");
    
    /* Test allocation wrappers */
    void *ptr1 = TOONc_malloc(100);
    ASSERT(ptr1 != NULL);
    
    void *ptr2 = TOONc_calloc(10, sizeof(int));
    ASSERT(ptr2 != NULL);
    
    /* Verify calloc zero-initialized */
    int *int_ptr = (int *)ptr2;
    for (int i = 0; i < 10; i++) {
        ASSERT(int_ptr[i] == 0);
    }
    
    /* Test realloc */
    void *ptr3 = TOONc_realloc(ptr1, 200);
    ASSERT(ptr3 != NULL);
    
    
    /* Test object tree freeing */
    const char *toon = 
        "complex:\n"
        "  array[3]: 1,2,3\n"
        "  nested:\n"
        "    deep: value\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT(root != NULL);
    
    /* This should not leak memory */
    TOONc_free(root);
    
    printf("✓ Memory management passed\n");
    return 0;
}

/* -----------------------------------------------------------------------------
 * Test 9: Type checking and getter macros
 * -------------------------------------------------------------------------- */
int test_type_checking() {
    TEST("Type checking and getter macros");
    
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
    ASSERT(root != NULL);
    
    /* Test type checking macros */
    toonObject *str_obj = TOONc_get(root, "string");
    ASSERT(TOON_IS_STRING(str_obj));
    ASSERT(!TOON_IS_INT(str_obj));
    
    toonObject *int_obj = TOONc_get(root, "integer");
    ASSERT(TOON_IS_INT(int_obj));
    ASSERT(!TOON_IS_STRING(int_obj));
    
    toonObject *dbl_obj = TOONc_get(root, "double");
    ASSERT(TOON_IS_DOUBLE(dbl_obj));
    
    toonObject *bool_true = TOONc_get(root, "bool_true");
    ASSERT(TOON_IS_BOOL(bool_true));
    ASSERT_EQ(TOON_GET_BOOL(bool_true), 1);
    
    toonObject *bool_false = TOONc_get(root, "bool_false");
    ASSERT(TOON_IS_BOOL(bool_false));
    ASSERT_EQ(TOON_GET_BOOL(bool_false), 0);
    
    toonObject *null_obj = TOONc_get(root, "null_val");
    ASSERT(TOON_IS_NULL(null_obj));
    
    toonObject *arr_obj = TOONc_get(root, "array");
    ASSERT(TOON_IS_LIST(arr_obj));
    
    toonObject *obj_obj = TOONc_get(root, "object");
    ASSERT(TOON_IS_OBJ(obj_obj));
    
    /* Test getter macros on wrong types (should return defaults) */
    ASSERT_EQ(TOON_GET_INT(str_obj), 0); /* String -> int returns 0 */
    ASSERT(TOON_GET_STRING(int_obj) == NULL); /* Int -> string returns NULL */
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(bool_true), 0.0, 0.0001); /* Bool -> double returns 0.0 */
    
    TOONc_free(root);
    printf("✓ Type checking passed\n");
    return 0;
}

/* -----------------------------------------------------------------------------
 * Test 10: Complex nested structure
 * -------------------------------------------------------------------------- */
int test_complex_structure() {
    TEST("Complex nested structure");
    
    const char *toon = 
        "# Complex configuration example\n"
        "app:\n"
        "  name: MyApp\n"
        "  version: 2.1.0\n"
        "  settings:\n"
        "    debug: true\n"
        "    log_level: verbose\n"
        "    features[3]: auth,api,ui\n"
        "\n"
        "database:\n"
        "  hosts[2]: db1.example.com,db2.example.com\n"
        "  config:\n"
        "    pool_size: 10\n"
        "    timeout: 30.5\n"
        "\n"
        "users[3]{id,name,roles}:\n"
        "  1,Admin,admin\n"
        "  2,User,user\n"
        "  3,Guest,\n";
    
    toonObject *root = TOONc_parseString(toon);
    ASSERT(root != NULL);
    
    /* Test various access patterns */
    toonObject *app_name = TOONc_get(root, "app.name");
    ASSERT(app_name != NULL);
    ASSERT_STR_EQ(TOON_GET_STRING(app_name), "MyApp");
    
    toonObject *debug = TOONc_get(root, "app.settings.debug");
    ASSERT(debug != NULL);
    ASSERT(TOON_IS_BOOL(debug));
    ASSERT_EQ(TOON_GET_BOOL(debug), 1);
    
    toonObject *features = TOONc_get(root, "app.settings.features");
    ASSERT(features != NULL);
    ASSERT(TOON_IS_LIST(features));
    ASSERT_EQ(TOONc_getArrayLength(features), 3);
    ASSERT_STR_EQ(TOON_GET_STRING(TOONc_getArrayItem(features, 0)), "auth");
    
    toonObject *hosts = TOONc_get(root, "database.hosts");
    ASSERT(hosts != NULL);
    ASSERT(TOON_IS_LIST(hosts));
    ASSERT_EQ(TOONc_getArrayLength(hosts), 2);
    ASSERT_STR_EQ(TOON_GET_STRING(TOONc_getArrayItem(hosts, 1)), "db2.example.com");
    
    toonObject *timeout = TOONc_get(root, "database.config.timeout");
    ASSERT(timeout != NULL);
    ASSERT(TOON_IS_DOUBLE(timeout));
    ASSERT_FLOAT_EQ(TOON_GET_DOUBLE(timeout), 30.5, 0.0001);
    
    /* Test tabular data */
    toonObject *users = TOONc_get(root, "users");
    ASSERT(users != NULL);
    ASSERT(TOON_IS_LIST(users));
    ASSERT_EQ(TOONc_getArrayLength(users), 3);
    
    toonObject *user2 = TOONc_getArrayItem(users, 1);
    ASSERT(user2 != NULL);
    
    toonObject *user2_name = TOONc_get(user2, "name");
    ASSERT(user2_name != NULL);
    ASSERT_STR_EQ(TOON_GET_STRING(user2_name), "User");
    
    toonObject *user3 = TOONc_getArrayItem(users, 2);
    ASSERT(user3 != NULL);
    
    toonObject *user3_roles = TOONc_get(user3, "roles");
    ASSERT(user3_roles != NULL);
    ASSERT(TOON_IS_NULL(user3_roles)); /* Empty cell becomes null */
    
    /* Print structure for visual verification */
    printf("\nComplex structure:\n");
    TOONc_printObject(root, 0);
    
    /* Test JSON conversion */
    printf("\nJSON output:\n");
    TOONc_toJSON(root, stdout, 0);
    printf("\n");
    
    TOONc_free(root);
    printf("✓ Complex structure passed\n");
    return 0;
}

/* -----------------------------------------------------------------------------
 * Test 11: File parsing
 * -------------------------------------------------------------------------- */
int test_file_parsing() {
    TEST("File parsing");
    
    /* Create a temporary file for testing */
    const char *test_content = 
        "test_file: true\n"
        "value: 123\n"
        "nested:\n"
        "  item: file_test\n";
    
    FILE *fp = tmpfile();
    ASSERT(fp != NULL);
    
    fwrite(test_content, 1, strlen(test_content), fp);
    rewind(fp);
    
    /* Parse from file */
    toonObject *root = TOONc_parseFile(fp);
    ASSERT(root != NULL);
    
    /* Verify parsed content */
    toonObject *test_file = TOONc_get(root, "test_file");
    ASSERT(test_file != NULL);
    ASSERT(TOON_IS_BOOL(test_file));
    ASSERT_EQ(TOON_GET_BOOL(test_file), 1);
    
    toonObject *value = TOONc_get(root, "value");
    ASSERT(value != NULL);
    ASSERT(TOON_IS_INT(value));
    ASSERT_EQ(TOON_GET_INT(value), 123);
    
    toonObject *item = TOONc_get(root, "nested.item");
    ASSERT(item != NULL);
    ASSERT(TOON_IS_STRING(item));
    ASSERT_STR_EQ(TOON_GET_STRING(item), "file_test");
    
    TOONc_free(root);
    
    printf("✓ File parsing passed\n");
    return 0;
}

/* -----------------------------------------------------------------------------
 * Test 12: Performance and stress test
 * -------------------------------------------------------------------------- */
int test_performance() {
    TEST("Performance and stress test");
    
    /* Create a large TOON document */
    char *large_toon = TOONc_malloc(1000000); /* 1MB document */
    char *ptr = large_toon;
    
    /* Header */
    ptr += sprintf(ptr, "large_dataset:\n");
    
    /* Create 1000 records */
    for (int i = 0; i < 1000; i++) {
        ptr += sprintf(ptr, "  record_%d:\n", i);
        ptr += sprintf(ptr, "    id: %d\n", i);
        ptr += sprintf(ptr, "    name: User%d\n", i);
        ptr += sprintf(ptr, "    value: %.2f\n", i * 1.5);
        ptr += sprintf(ptr, "    active: %s\n", i % 2 == 0 ? "true" : "false");
    }
    
    /* Add a large array */
    ptr += sprintf(ptr, "large_array[1000]:\n");
    for (int i = 0; i < 1000; i++) {
        ptr += sprintf(ptr, "  %d", i);
        if (i < 999) ptr += sprintf(ptr, ",");
        if (i % 10 == 9) ptr += sprintf(ptr, "\n");
    }
    
    /* Parse the large document */
    clock_t start = clock();
    toonObject *root = TOONc_parseString(large_toon);
    clock_t end = clock();
    
    ASSERT(root != NULL);
    
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Parsed 1MB document in %.3f seconds\n", elapsed);
    
    /* Verify some data */
    toonObject *record_500 = TOONc_get(root, "large_dataset.record_500");
    ASSERT(record_500 != NULL);
    
    toonObject *record_500_id = TOONc_get(record_500, "id");
    ASSERT(record_500_id != NULL);
    ASSERT_EQ(TOON_GET_INT(record_500_id), 500);
    
    toonObject *large_array = TOONc_get(root, "large_array");
    ASSERT(large_array != NULL);
    ASSERT(TOON_IS_LIST(large_array));
    ASSERT(TOONc_getArrayLength(large_array) >= 1000);
    
    /* Clean up */
    TOONc_free(root);
    free(large_toon);
    
    printf("✓ Performance test passed\n");
    return 0;
}

/* -----------------------------------------------------------------------------
 * Main test runner
 * -------------------------------------------------------------------------- */
int main() {
    printf("Starting TOONc test suite\n");
    printf("===========================\n");
    
    int failures = 0;
    
    /* Run all tests */
    failures += test_basic_parsing();
    failures += test_nested_objects();
    failures += test_simple_arrays();
    failures += test_tabular_data();
    failures += test_comments_and_whitespace();
    failures += test_edge_cases();
    failures += test_object_creation();
    failures += test_memory_management();
    failures += test_type_checking();
    failures += test_complex_structure();
    failures += test_file_parsing();
    // Comment out performance test for quick runs
    // failures += test_performance();
    
    printf("\n===========================\n");
    if (failures == 0) {
        printf("All tests PASSED! ✓\n");
        return 0;
    } else {
        printf("%d test(s) FAILED!\n", failures);
        return 1;
    }
}