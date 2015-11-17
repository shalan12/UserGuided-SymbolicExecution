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
	a = a + 10; // a = A+10
	b = b + 5; // b = B + 5
	int d = 1;
	int c = 0;
	if (a > b)
	{
		c = b + 1; // c = B + 5 +1
		b = b + a; // b =  B + 5 + A + 10
		d = 2+c;
		if (b > 2) // (B+5 + A+10) > 2
		{
			b = b + 9; // b= B+5+A+10+9
			c = 11; 
			d = b + a; // d = B+5+A+10+9 + A + 10 
		}
		else
		{
			c = 5;
		}
		a = a + 1; // a = A + 10 + 1
		b = a + d; // b = A + 10 + 1 + B+5+A+10+9 + A + 10 
		c = d + a; // c = B+5+A+10+9 + A + 10 + A + 10 + 1
		d = c + d; // d = B+5+A+10+9 + A + 10 + A + 10 + 1 + B+5+A+10+9 + A + 10
		if ((c + d) < 10) // (B+5+A+10+9 + A + 10 + A + 10 + 1) +
						  // (B+5+A+10+9 + A + 10 + A + 10 + 1) + 
						  // (B+5+A+10+9 + A + 10)
		{
			a = a + 5;
		}
		else
		{
			b = b + 5;
		}
	}
	else
	{
		b = b + 2;
		d = b;
		c = d + b;
		if (a < d)
		{
			c = c + d + b;
		}
		else
		{
			c = c + d;
			a = a + 5;
		}
		a = a + 50;
		b = c + b + d;
	}
	a = b + d;
}


/*
void notmain(int a, int b)
{
	if (a > b)
	{
		if(a+1 > b)
			b = b + 1;
		else
			b= b+2;
	}
	else
	{
		b = b + 2;
	}
}*/