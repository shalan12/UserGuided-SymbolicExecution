#include <stdlib.h>
int bar(int c, int d);
int ext(int x);
void notmain(int a, int b)
{
  int c = 10;
  if (a > c)
  {
    if(a+1 > c)
    {
      b = bar(a+1,b+2);
    }
    else
    {
      b = b;
    }
  }
  else
  {
    int x = 3;
    while(a > 0)
    {
      a--;
      x = x*2;
    }
    b = b + ext(a);
  }
}
int bar(int c, int d)
{
  if (c > d)
    return c*d;
  else return c+d;
}
