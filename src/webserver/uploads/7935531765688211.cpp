#include <stdlib.h>
int bar(int c, int d);
void notmain(int a, int b)
{
  if (a > b)
  {
    if(a+1 > b)
    {
      b = bar(a+1,b+2);
      b = b + abs(a);
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