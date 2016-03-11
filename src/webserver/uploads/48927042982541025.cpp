#include <stdlib.h>
int bar(int c, int d);
void notmain(int a, int b)
{
  int c = 10;
  if (a > c)
  {
    if(a+1 > c)
    {
      b = bar(a+1,b+2);
      b = b + abs(a-b);
    }
    else
    {
      b = b + 1;
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
int bar(int c, int d)
{
  return c*d;
}