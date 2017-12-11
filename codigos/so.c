int execucao[3];
int pc[3];
int maxProgram;
int programAtual;


int input(void)
{
}

void output(int out)
{
}

void alteraContexto(int i){
  /* carrega os registradores e altera o contexto global */
  pc[programAtual] = 123; /* alterar para armazenar o PC do reg30 aqui */
  programAtual = i;
  pc[programAtual] = 456; /* alterar pra dar jump no i = pc[programAtual] */
  programAtual = 666;
  programAtual = 666;
  programAtual = 666;
  programAtual = 666;
  programAtual = 666;
  programAtual = 666;
  programAtual = 666;
  programAtual = 666;
  programAtual = 666;
  programAtual = 666;
  programAtual = 666;

}

void buscaPrograma(void){
  int i;
  int found;
  found = 0;
  i = programAtual;
  i = i + 1;
  if(i > maxProgram)
    i = 1;
  while(found == 0){
    /* procura o proximo programa*/
    if(execucao[i] == 1){
      alteraContexto(i);
    }
    i = i + 1;
    if(i > maxProgram)
      i = 1;
  }
}


void main(void)
{
  maxProgram = 2;
  programAtual = 1;
  execucao[0] = 1;
  execucao[1] = 1;
  execucao[2] = 1;
  pc[0] = 0;
  pc[1] = 0;
  pc[2] = 0;
    /*a partir daqui, salva o programa*/
  pc[programAtual] = 33;
  buscaPrograma();
}
