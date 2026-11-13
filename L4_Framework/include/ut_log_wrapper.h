/**
 * @file ut_log_wrapper.h
 * @brief Wrapper for UT_LOG macro with fallback implementation
 * 
 * Ensures UT_LOG is always available, even if ut.h doesn't define it
 */

#ifndef UT_LOG_WRAPPER_H
#define UT_LOG_WRAPPER_H

#include <cstdio>

// Include ut.h for UT_LOG, UT_init, UT_run_tests, etc.
// With proper CUnit headers in include path, ut_cunit.h will resolve correctly
// Wrap in extern "C" to prevent C++ name mangling of C library functions

#ifdef __cplusplus
extern "C" {
#endif
#include <ut.h>
#ifdef __cplusplus
}
#endif


// If UT_LOG not defined, provide fallback implementation
#ifndef UT_LOG
#define UT_LOG(fmt, ...) \
    do { \
        fprintf(stdout, "[UT] " fmt, ##__VA_ARGS__); \
        fputc('\n', stdout); \
        fflush(stdout); \
    } while(0)
#endif

#endif // UT_LOG_WRAPPER_H
