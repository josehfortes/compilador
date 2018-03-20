int execucao[12]; /*salva todos os programas em execucao ou nao */
int maxProgram;
int programAtual;
int proxPrograma;

int input(void)
{
}

void output(int out)
{
}

int buscaPrograma(void){
  int i;
  int found;
  found = 0;
  i = programAtual;
  i = i + 1; /* i recebe o programa atual +1 */
  if(i > maxProgram)
    i = 1;
  while(found == 0){
    /* procura o proximo programa*/
    if(execucao[i] == 1){
      proxPrograma = i;
	  return i;
    }
    i = i + 1;
    if(i > maxProgram)
      i = 1; /* volta para o 1 - inicio dos programsa */
  }
}




void main(void)
{
  int retPrograma;
  int getMenu;
  maxProgram = 962;
  programAtual = 0;
  retPrograma = 0;
  execucao[1] = 1;
  execucao[2] = 1;
  execucao[3] = 1;
  execucao[4] = 1;
  execucao[5] = 1;
  execucao[6] = 1;
  execucao[7] = 1;
  execucao[8] = 1;
  execucao[9] = 1;
  execucao[10] = 1;
  execucao[11] = 1;
  output(99);
  getMenu = input();
  while (getMenu != 16){ /*se for 16 inicia a execucao */
	  output(10);
	  if(getMenu == 1){ /*seleciona quais programas quer executar - 16 sai da selecao */
		getMenu = input();
		while( getMenu != 16 ){
		  if(getMenu <= maxProgram){
			  execucao[getMenu] = 1;
			  output(getMenu);
		  }
		  output(11);
		  getMenu = input();
		}
	  }
	  if(getMenu == 2){
		  
		  output(22);
		  execucao[1] = 1;
		  execucao[2] = 1;
		  execucao[3] = 1;
		  execucao[4] = 1;
		  execucao[5] = 1;
		  execucao[6] = 1;
		  execucao[7] = 1;
		  execucao[8] = 1;
		  execucao[9] = 1;
		  execucao[10] = 1;
		  execucao[11] = 1;
	  }
	  if (getMenu == 3){
		  output(33);
		  execucao[1] = 0;
		  execucao[2] = 0;
		  execucao[3] = 0;
		  execucao[4] = 0;
		  execucao[5] = 0;
		  execucao[6] = 0;
		  execucao[7] = 0;
		  execucao[8] = 0;
		  execucao[9] = 0;
		  execucao[10] = 0;
		  execucao[11] = 0;
	  }
	  if(getMenu == 4){ /*seleciona quais programas quer remover da execucao - 16 sai da selecao */
		getMenu = input();
		while( getMenu != 16 ){
		  if(getMenu <= maxProgram){
			  execucao[getMenu] = 0;
			  output(getMenu);
		  }
		  output(44);
		  getMenu = input();
		}
	  }
	  output(99);
	  getMenu = input();
  }
  /*parte que SEMPRE se repete*/
  retPrograma = buscaPrograma();
  programAtual = retPrograma;
  
}
