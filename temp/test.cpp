#include <iostream>
using namespace std;
int main()
{

	string* x = new string();
	cout << (int)x;
	cout << &(*x);
}