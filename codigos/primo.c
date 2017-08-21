int input(void)
{
}

void output(int out)
{
}

int primo(int p)
{
  int i;
  int res;
  i = p - 1;
  while(i > 1){
    res = p / i;
    res = res * i;
    if (res == p)
      return 2;
    i = i - 1;
  }
  return 1;
}

void main(void)
{
    int x;
    x = input();
    x = primo(x);
    output(x);
}
