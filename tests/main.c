#import "libtest.h"


void phase_1_tests(void);
void internal_ops_test(void);
void phase_2_tests(void);
void phase_3_tests(void);

void global_tests(void);

int main(int argc, char ** argv)
{
	
	lt_init(argc, argv);
	
	internal_ops_tests();
	phase_1_tests();
	phase_2_tests();
	phase_3_tests();
	global_tests();
	
	lt_printresults();
	return !lt_allpass();
}