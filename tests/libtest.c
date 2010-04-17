
#include "libtest.h"
#include <stdlib.h>

#include "setjmp.h"
#include <stdio.h>
#include "string.h"
struct err_list {
	char * test_name;
	char * file;
	int line;
	char * assertion_failure;
	struct err_list * next;
};

char * g_only_name;
static jmp_buf g_test_return;
static char * g_test_name;
static struct err_list * g_err_head;

static int g_test_count = 0;

void lt_append_err(char * file, int line, char * failure)
{
	struct err_list ** i = &g_err_head;
	
	while (*i)
		i = &(*i)->next;
	
	*i = (struct err_list* )malloc(sizeof(struct err_list));
	(*i)->next = NULL;
	(*i)->test_name = g_test_name;
	(*i)->file = file;
	(*i)->line = line;
	(*i)->assertion_failure = failure;
}

void lt_init(int argc, char ** argv)
{
	g_only_name = NULL;
	if (argc == 2)
		g_only_name = argv[1];
	
	g_err_head = NULL;
}

void lt_invokeTest(char * testname, test_fn suite)
{
	if (g_only_name && strcmp(g_only_name, testname) != 0)
		return;
	
	g_test_name = testname;
	if (!setjmp(g_test_return))
		suite();
	
	g_test_name = NULL;
	g_test_count ++;
}

void lt_assert(char * file, int line, char * text, bool value)
{
	if (!value) lt_append_err(file, line, text);
}

void lt_require(char * file, int line, char * text, bool value)
{
	if (!value){
		lt_append_err(file, line, text);
		longjmp(g_test_return, 1);
	}
}

void lt_printresults()
{
	if (g_err_head)
		printf("Ran %d tests:\n", g_test_count);
	else
		printf("Ran %d tests: ALL OK\n", g_test_count);
	
	struct err_list *i = g_err_head;
	
	while (i)
	{
		struct err_list * n = i;
		printf("%20s:%-4d\t\t%s: ASSERTION FAILED: %s\n", n->file, n->line, n->test_name, n->assertion_failure);
		i = i->next;
		free(n);
	}
}
