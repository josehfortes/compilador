int input(void)
{
}

void output(int out)
{
}

int parimpar(int x)
{
  while(x >= 2){
    x = x - 2;
  }
  return x;
}

void main(void)
{
    int a;
    a = input();
    a = parimpar(a);
    output(a);
}
