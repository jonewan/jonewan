#include <stdio.h>
#include <ctype.h>

#include "jonewan.h"

boolean isNumberic(const int ch){
	return isdigit(ch) || isSign(ch) || isDot(ch);
}

const char *skipBlank(const char *str){
	while(0 != *str && isspace(*str))
		str++;

	return str;
}

boolean isSign(const int ch){
	return '+' == ch || '-' == ch;
}

boolean isDot(const int ch){
	return '.' == ch;
}

