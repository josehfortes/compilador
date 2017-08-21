int input(void)
{
}

void output(int out)
{
}

int num[4];

int max(void)
{
  int m;
  int i;
  m = 0;
  i = 0;
  while (i <= 3){
    if(m < num[i])
      m = num[i];
    i = i + 1;
  }
  return m;
}

void main(void)
{
    int a;
    num[0] = 5;
    num[1] = 9;
    num[2] = 21;
    num[3] = 8;
    a = max();
    output(a);
}
