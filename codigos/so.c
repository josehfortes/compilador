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
  maxProgram = 44;
  programAtual = 0;
  retPrograma = 0;
  execucao[1] = 0;
  execucao[2] = 0;
  execucao[3] = 0;
  execucao[4] = 0;
  getMenu = 0;
  while (getMenu != 16){ /*se for 16 inicia a execucao */

	  if(getMenu == 1){ /*seleciona quais programas quer executar - 16 sai da selecao */
		getMenu = input();
		maxProgram = 9876;
		  if(getMenu <= maxProgram){
			  execucao[getMenu] = 1;
		  }
	  }
	  if(getMenu == 2){
		  execucao[1] = 1;
		  execucao[2] = 1;
		  execucao[3] = 1;
		  execucao[4] = 1;
	  }
	  if (getMenu == 3){
		  execucao[1] = 0;
		  execucao[2] = 0;
		  execucao[3] = 0;
		  execucao[4] = 0;
	  }
	  if(getMenu == 4){ /*seleciona quais programas quer remover da execucao - 16 sai da selecao */
		getMenu = input();
		maxProgram = 9876;
		  if(getMenu <= maxProgram){
			  execucao[getMenu] = 0;
		  }
	  }
	  getMenu = input();
	  maxProgram = 9876;
  }
  /*parte que SEMPRE se repete*/
  if(execucao[0] == 0){
	  if(execucao[1] == 0){
		  if(execucao[2] == 0){
			  if(execucao[3] == 0){
				  getMenu = 666;
			  }
		  }
	  }
  }
  
  getMenu = 111;
  execucao[programAtual] = 24;
  getMenu = 222;
	
  getMenu = 999;
  retPrograma = buscaPrograma();
  programAtual = retPrograma;
  getMenu = 888;
}
