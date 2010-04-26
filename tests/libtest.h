#ifndef _LIBTEST_H_
#define _LIBTEST_H_

#include <stdbool.h>

typedef void (*test_fn)(void);

void lt_init(int argc, char ** argv);

void lt_invokeTest(char * name, test_fn suite);
void lt_assert(char * file, int line, char * text, bool value);
void lt_require(char * file, int line, char * text, bool value);

void lt_printresults();
bool lt_allpass(void);

#define LT_ASSERT(x) lt_assert(__FILE__,__LINE__,#x, x)
#define LT_REQUIRE(x) lt_require(__FILE__,__LINE__,#x, x)
#define LT_THROW(x) lt_require(__FILE__,__LINE__,x, false)
#define _T(x) lt_invokeTest(#x, x)
#endif