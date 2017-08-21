int input(void)
{
}

void output(int out)
{
}

int fatorial(int x)
{
  int fat;
  fat = 1;
  while(x > 0){
    fat = fat * x;
    x = x - 1;
  }
  return fat;
}

void main(void)
{
    int a;
    a = input();
    a = fatorial(a);
    output(a);
}
