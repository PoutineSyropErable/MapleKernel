#include <stdio.h>

// static data
const int stat_var = 25;
const char word1[50] = "banana";
const char word2[50] = "123";

// changing data
int change = 32;

// code
void funca() {

	printf("%s, %d\n", word1, stat_var);
	printf("%s", word2);
	// equiv vv
	printf("banana, %d\n", stat_var);
	printf("123");
	change = 27;
}
