#include <stdlib.h>     /* abs */
int notmain2(int c, int d);
void notmain(int a, int b, int c)
{
	a = a + b;
	b = a - b;
	a = a - b; 
	if(a < b){


	}else {
		a = a +1;
		b = b +1;

	}
	if( a < c ){
		b = a;

	} else {

		c = a;
	}
}
int notmain2(int c, int d)
{
	if(c > d)
		return c+d;
	else
		return c-d;
}