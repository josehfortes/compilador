int input(void)
{
}

void output(int out)
{
}

int divisible(int z)
{
  while(z >= 7){
    z = z - 7;
  }
    if (z == 0)
        return 1;
    else
        return 0;
}

void main(void)
{
    int number;
    number = input();
    number = divisible(number);
    output(number);
}
