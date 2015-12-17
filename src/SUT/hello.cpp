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
			b = b+2;
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