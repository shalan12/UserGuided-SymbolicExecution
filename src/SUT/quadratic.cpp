#include <iostream>
int sqrt(int b)
{
	
}

void notmain(int a, int b, int c)
{
	int b_4ac = b*b-4*a*c;
	std::cout << "root # 1" << (-1*b+sqrt(b_4ac))/(2*a) << std::endl;
	std::cout << "root # 2" << (-1*b-sqrt(b_4ac))/(2*a) << std::endl;
}