#include <stdlib.h>     /* abs */
int notmain2(int c, int d);
void notmain(int a, int b)
{
	if (a > b)
	{
		if(a+1 > b)
		{
			b = notmain2(a+1,b+2);
			b = b + abs(a);
		}
		else
		{
			b = b + 1;
		}
	}
	else
	{
		int x = 3;
		while(x > 0)
		{
			a++;
			x--;
		}
		b = b + 2;
	}
}
int notmain2(int c, int d)
{
	if(c > d)
		return c+d;
	else
		return c-d;
}