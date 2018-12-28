#ifndef _JONEWAN_H_
#define _JONEWAN_H_

#define GET_BYTE(vbyte, index)      ((vbyte) & (1 << ((index) ^ 7)) != 0)
#define	SET_BYTE(vbyte, index)		((vbyte) |= (1 << ((index ^ 7)))
#define CLR_BYTE(vbyte, index)		((vbyte) &= (~(1 << ((index) ^ 7))))

typedef unsigned char	boolean;
typedef	boolean 	u8;
#define	TRUE		1
#define	FALSE		0

#define	NOT_FOUND	-1

const char *skipBlank(const char *str);
boolean isSign(const int ch);
boolean isDot(const int ch);
boolean isNumberic(const int ch);
void showIArray(int *arr, int count);
int *makeRandomIntArray(int count, int min, int max);

#endif
