#ifndef _LIBTEST_H_
#define _LIBTEST_H_

#include <stdbool.h>
#include <string.h>

typedef void (*test_fn)(void);

void lt_init(int argc, char ** argv);

void lt_invokeTest(char * name, test_fn suite);
void lt_assert(char * file, int line, char * text, bool value);
void lt_require(char * file, int line, char * text, bool value);

void * lt_logged_malloc(char * file, int line, size_t size);
void lt_logged_free(char * file, int line, void * ptr);

void lt_printresults();
bool lt_allpass(void);

#define LT_ASSERT(x) lt_assert(__FILE__,__LINE__,#x, x)
#define LT_REQUIRE(x) lt_require(__FILE__,__LINE__,#x, x)
#define LT_THROW(x) lt_require(__FILE__,__LINE__,x, false)

#define LT_LOGGED_MALLOC(x) lt_logged_malloc(__FILE__, __LINE__, x)
#define LT_LOGGED_FREE(x) lt_logged_free(__FILE__, __LINE__, x)

#define _T(x) lt_invokeTest(#x, x)
#endif