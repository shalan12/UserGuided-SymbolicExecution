int notmain(int p)
{
	for (int i = 2; i < 10; i++)
	{
		if ((p / i) == 1)
			return 0;
	}

	return 1;
}