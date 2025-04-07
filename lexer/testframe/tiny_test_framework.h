// test/tiny_test_framework.h
#ifndef TINY_TEST_FRAMEWORK_H
#define TINY_TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h> // For strcmp in ASSERT_STR_EQ

// --- Test Function Type ---
typedef void (*TestFunc)(void);

// --- Test Registration ---
void register_test(const char* test_name, TestFunc func);

// --- Test Runner ---
// Returns the number of failed tests
int run_all_tests(void);

// --- Assertion Macros ---
// Internal helper - DO NOT CALL DIRECTLY
void _test_fail(const char* expression, const char* file, int line, const char* message);

// Keep track of failures within the current test function
extern int g_current_test_failed; // Defined in .c file

#define ASSERT_MSG(condition, message) \
    do { \
        if (!(condition)) { \
            _test_fail(#condition, __FILE__, __LINE__, message); \
            g_current_test_failed++; \
        } \
    } while (0)

#define ASSERT_TRUE(condition) ASSERT_MSG(condition, NULL)
#define ASSERT_FALSE(condition) ASSERT_MSG(!(condition), NULL)
#define ASSERT_NULL(ptr) ASSERT_MSG((ptr) == NULL, "Pointer was not NULL")
#define ASSERT_NOT_NULL(ptr) ASSERT_MSG((ptr) != NULL, "Pointer was NULL")

// Basic type comparisons (add more as needed: float, double, etc.)
#define ASSERT_EQ_INT(expected, actual) \
    do { \
        int _exp = (expected); int _act = (actual); \
        if (_exp != _act) { \
            char _msg[128]; \
            snprintf(_msg, sizeof(_msg), "Expected %d, but got %d", _exp, _act); \
            ASSERT_MSG(0, _msg); /* Force failure with message */ \
        } \
    } while (0)

#define ASSERT_EQ_CHAR(expected, actual) \
    do { \
        char _exp = (expected); char _act = (actual); \
        if (_exp != _act) { \
            char _msg[128]; \
            snprintf(_msg, sizeof(_msg), "Expected '%c' (%d), but got '%c' (%d)", _exp, _exp, _act, _act); \
            ASSERT_MSG(0, _msg); \
        } \
    } while (0)

#define ASSERT_EQ_SIZE(expected, actual) \
    do { \
        size_t _exp = (expected); size_t _act = (actual); \
        if (_exp != _act) { \
            char _msg[128]; \
            snprintf(_msg, sizeof(_msg), "Expected %zu, but got %zu", _exp, _act); \
            ASSERT_MSG(0, _msg); \
        } \
    } while (0)


#define ASSERT_STR_EQ(expected, actual) \
    do { \
        const char* _exp = (expected); const char* _act = (actual); \
        if (_exp == NULL || _act == NULL) { \
            if (_exp != _act) { /* Only fail if one is NULL and other isn't */ \
                 ASSERT_MSG(0, "String comparison failed: one pointer is NULL, the other is not."); \
            } \
        } else if (strcmp(_exp, _act) != 0) { \
            char _msg[256]; \
            snprintf(_msg, sizeof(_msg), "Expected \"%s\", but got \"%s\"", _exp, _act); \
            ASSERT_MSG(0, _msg); \
        } \
    } while (0)


// --- Test Suite Registration (Optional but good practice) ---
// Declare functions that register tests for each module
void register_arena_tests(void);
void register_lexer_tests(void);

#endif // TINY_TEST_FRAMEWORK_H