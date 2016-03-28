int notmain(int p)
{
	for (int i = 0; i < 10; i++)
	{
		if (p % i == 0)
			return 0;
	}
	return 1;
}