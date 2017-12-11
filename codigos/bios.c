int input(void)
{
}

void output(int out)
{
}

int max[12];

void main(void)
{
  int qtprogram;
  int qtprogrami;
  int i;
  int posicao;
  int contexto;
  qtprogram = 2;
  max[0] = 2; /* fatorial */
  max[1] = 3; /* divisivel sete */
  qtprogrami = 0;
  while(qtprogrami < qtprogram){
    /* passa por todos os programas */
    i = 0;
    while(i < max[qtprogrami]){
      /* copia o programa */
      /*posicao = i;
      contexto = qtprogrami;*/
      i = qtprogrami;
      qtprogrami = i;
      i = 999;
      i = i + 1;
    }
    qtprogrami = qtprogrami + 1;
  }
}
