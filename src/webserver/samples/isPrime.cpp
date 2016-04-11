int notmain(int p)
{
	int i = 2;
	while (i < 10)
	{
		if ((p / i) == 1)
			return 0;
		else
			i++;
	}

	return 1;
}