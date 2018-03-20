int input(void)
{
}

void output(int out)
{
}

int potencia(int x)
{
  int val;
  val = 2;
  while(x > 1){
    val = 2 * val;
    x = x - 1;
  }
  return val;
}

void main(void)
{
    int a;
    a = input();
	output(a);
    a = potencia(a);
    output(a);
}
