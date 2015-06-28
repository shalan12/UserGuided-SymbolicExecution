void foo(int a, int b)
{
	a = a + 10;
	b = b + 5;
	int d;
	int c;
	if(a > b)
	{
		c = b + 1;
		b = b + a;
<<<<<<< HEAD
		d = 2;
		if ( b > 2 )
=======
		d = 2*e;
		if (b > 2)
>>>>>>> 6a00576535cedd3a31676faa6625e6ca8b60b39f
		{
			b = b + 9;
			c = 11;
			d = b + a;
		}
		else
		{
			c = 5;
		}
		a = a + 1;
		b = a + d;
		c = d + a;
		d = c * d;
		if ((c + d) < 10)
		{
			a = a / 5;
		}
		else
		{
			b = b / 5;
		}
		a = c + d;
	}
	else
	{
		b = b * 2;
		d = b;
		c = d + b;
		if (a < d)
		{
			c = c * d / b;
		}
		else
		{
			c = c + d;
			a = a + 5;
		}
		a = a + 50;
		b = c + b + d;
	}
	a = a + b;
	c = c + d;
}
<<<<<<< HEAD
// int main()
// {
// 	foo(50,100);
// }
=======
int main()
{
	foo(50,100);
}
int e = 4;
>>>>>>> 6a00576535cedd3a31676faa6625e6ca8b60b39f
