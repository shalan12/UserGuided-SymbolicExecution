#include <iostream>
int notmain2(int c, int d);
int main()
{
	int a = 10;
	int b = 5;
	if (a > b)
	{
		b = b + 1;
	}
	else
	{
		b = b + 2;
	}
	
}

void notmain(int a, int b)
{
	if (a > b)
	{
		if(a+1 > b)
			b = b + 1;
		else
		{
			b = notmain2(a+1,b+2);
			//std::cout << b;
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
	return c+d;
}