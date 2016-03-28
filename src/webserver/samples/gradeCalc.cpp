int notmain(int percentage)
{
	int grade;
	if (percentage > 80)
	{
		grade = 'A';
	}
	else if (percentage > 70)
	{
		grade = 'B';
	}
	else if (percentage > 60)
	{
		grade = 'C';
	}
	else if (percentage > 50)
	{
		grade = 'D';
	}
	else
	{
		grade = 'F';
	}
	return grade;
}