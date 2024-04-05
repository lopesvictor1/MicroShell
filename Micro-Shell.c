//Sistemas Operacionais: Mini-Shell; Ariel Góes de Castro (1701450064); Victor Hugo Schneider Lopes (161150749).

//Bibliotecas necessárias
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> //includes: chdir, ...
#include <sys/types.h>
#include <sys/wait.h>

//Alguns buffers e variáveis globais
#define READ_BUFSIZE 1024
#define ARG_BUFSIZE 32
char *shell_commands[] = {"cd", "help", "exit", "pwd"};
unsigned int size_commands = sizeof(shell_commands) / sizeof(char *);

//Protótipos das funções
void shell_loop(char** envp); //Loop principal do shell
char* shell_read_line(); //funcao que le cada comando digitado pelo usuario
char** shell_split(char* line); //recebe o comando, e o divide em "tokens";
int shell_apply(char** args, char** envp); //verifica os tokens e chama as funções que irao executa-los
int shell_launch_std(char** args, char** envp); //executa comandos utilizando-se da funcao execv, execp ...
int shell_launch(int command, char** args); //executa comandos utilizando funcoes criadas nesse codigo
int shell_cd(char** args); //utiliza chdir para mudar o diretorio
int shell_help(); //apresenta uma mensagem de ajuda ao usuario
int shell_exit(); //fecha o programa
int shell_pwd(); // mostra o caminho ate o diretorio atual

//Função principal
int main(int argc, char** argv, char** envp){
  shell_loop(envp);
  return 0;
}

//Função que contém o loop principal do shell
void shell_loop(char** envp){
  char *line;
  char **args;
  int status;

  do{
    printf(">11 ");
    line = shell_read_line();
    args = shell_split(line);
    status = shell_apply(args, envp); //pode chamar "shell_launch" our "shell_launch_std"
    free(line);
    free(args);
  }while(status == 1);
}


//Função que lê a linha de comando digitada pelo usuário
char* shell_read_line(){
  unsigned int pos = 0;
  char* buffer = (char *)malloc(sizeof(char) * READ_BUFSIZE);
  int bufsize = READ_BUFSIZE;

  if(buffer == NULL){
    fprintf(stderr, "ERROR! BAD ALLOC! (buffer)\n");
    exit(EXIT_FAILURE);
  }

  char c;

  while (true){
    c = getchar();
    if (c == EOF){ // comando padrão de saída UNIX/LINUX: CTRL + D;
      exit(EXIT_SUCCESS);
    }
    else if (c == '\n'){ //quando o usuário digitar 'ENTER';
      buffer[pos] = '\0';
      break;
    }
    else{ //quando digitado um caractere válido, ele é atribuído ao buffer
      buffer[pos] = c;
    }
    pos++; //se a posição extrapolar o tamanho do buffer, é realocado o mais 1024 bytes

    if(pos >= READ_BUFSIZE){
      bufsize += READ_BUFSIZE;
      buffer = realloc(buffer, bufsize);

    if(buffer == NULL){
      fprintf(stderr, "ERROR! BAD REALLOC!\n");
        exit(EXIT_FAILURE);
    }
  }
}

  //Quando terminado, retorna o buffer contendo a linha de comando
  return buffer;
}


//Função que divide a linha de comando em tokens
char** shell_split(char* line){

  /*string de delimitadores (separam comandos da linha de comando informada)*/
  const char str_delim[2] = " \t";
  char *token;

  /*pega a primeira substring */
  token = strtok(line, str_delim);

  /*vetor que irá conter os argumentos do comando a ser executado, nesse caso podem haver 15 argumentos*/
  unsigned int argsize = ARG_BUFSIZE;
  char **args = (char**)malloc(sizeof(char*) * ARG_BUFSIZE);

  /*Caso não consiga alocar espaço para o char** args*/
  if(args == NULL){
    fprintf(stderr, "ERROR! BAD ALLOC! (args)\n");
    exit(EXIT_FAILURE);
  }

  int pos = 0;

  /* pega o resto das substrings */
  while(token != NULL){
    /*cada token contem um argumento, que é inserido no vetor de argumentos*/
    args[pos] = token;
    pos++;

    /*Se exis*/
    if(pos >= argsize){
      argsize += ARG_BUFSIZE;
      args = realloc(args, argsize);

      if(!args){
        fprintf(stderr, "ERROR! BAD ALLOC! (args)\n");
        exit(EXIT_FAILURE);
      }
    }

    /*renova a substring*/
    token = strtok(NULL, str_delim);
  }

  /*As chamadas de sistema execv, execp ... recebem o vetor de argumentos sempre terminado em NULL*/
  args[pos] = '\0'; //pode ser também 0 ou NULL

  return args;
}

//Função que aplica os comandos
int shell_apply(char** args, char** envp){

  //Verifica se foi passado algum comando, do contrário, continua rodando o shell;
  if(args[0] == NULL){
    return 1;
  }

  for(int i = 0; i < size_commands; i++){
    if(strcmp(args[0], shell_commands[i]) == 0){
      //perror("shell_aplly");
      return shell_launch(i, args); //comandos escritos nesse código;
    }
  }

  //senão...
  return shell_launch_std(args, envp); //comandos padrões por "execvp, exec..."
}

//Função que executa comandos escritos nesse código
int shell_launch(int command, char** args){
  if(command == 0){
    return shell_cd(args);
  }
  else if(command == 1){
    return shell_help();
  }
  else if(command == 2){
    return shell_exit();
  }
  else if(command == 3){
    return shell_pwd();
  }
  else{
    return 0;
  }
}

//Função que executa comandos padrões
int shell_launch_std(char** args, char** envp){

  int status;
  pid_t pid;
  pid = fork();

  if(pid == 0){ //se criar o processo filho, execute...
    if(execvpe(args[0], args, envp) == -1){
      perror("lauch_std");
    }
    exit(EXIT_FAILURE);
  }
  else if(pid < 0){ //se o processo filho não for criado corretamente;
    perror("launch_std");
  }
  else{ //
    do{
      waitpid(pid, &status, WUNTRACED);
    }while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

//Função que muda o diretório
int shell_cd(char** args){
  if(args[1] == NULL){
    printf("ERROR! MISSING ARGUMENTS!\n");
  }
  else{
    if(chdir(args[1]) != 0){
      perror("shell_cd");
    }
  }
  return 1;
}

//Função que apresenta uma mensagem de ajuda ao usuário
int shell_help(){
  printf("UNIPAMPA ALEGRETE - Ciencia da Computacao\n");
  printf("Micro-Shell por Victor Lopes e Ariel Góes de Castro\n");
  printf("Primeiro Trabalho da Disciplina de Sistemas Operacionais\n");
  printf("Comandos implementados pelo programa:\n");
  for(int i = 0; i < size_commands; i++){
    printf("%s ", shell_commands[i]);
  }
  printf("\nApenas digite o comando desejado e tecle ENTER\n");
  printf("Para obter informacoes sobre outros programas utilize o comando 'man'\n");

}

//Função que fecha o programa
int shell_exit(){
  return 0;
}

//Função que mostra o caminho até o diretório atual
int shell_pwd(){

  char s[200];

  if(getcwd(s, sizeof(s)) == NULL){
    perror("shell_pwd ERROR");
    return 0;
  }

  printf("%s\n", getcwd(s, sizeof(s)));
  return 1;
}
