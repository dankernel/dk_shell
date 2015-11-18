/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015년 11월 18일 16시 59분 22초
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<wait.h>

#define MAX 100

#define RED_R 1
#define RED_L 2
#define PIPE 3
#define CD 4
#define HELP 5

int get_token(char *buf, char **arg1, char **arg2){

  int let = 0;
  int i = 0;
  int ret = 0;
  char *tmp;

  tmp = strtok(buf, " \n");
  arg1[i] = tmp;

  i++;

  if( strstr(tmp, "help") ){
    i = 0;
    let = 1;
    ret = HELP;
  }else if( strstr(tmp, "cd") ){
    i = 0;
    let = 1;
    ret = CD;
  }

  while(tmp){
    tmp = strtok(NULL, " \n");
    if(!tmp)
      return ret;

    if( strstr(tmp, "<") ){
      i = 0;
      let = 1;
      ret = RED_L;
    }else if( strstr(tmp, ">") ){
      i = 0;
      let = 1;
      ret  = RED_R;
    }else if( strstr(tmp, "|") ){
      i = 0;
      let = 1;
      ret = PIPE;
    }else{

      if(let == 0)
        arg1[i] = tmp;
      else if(let == 1)
        arg2[i] = tmp;

      i++;
    }

  }

  return ret;
}

int do_cmd(char **arg){
  execvp(*arg, arg);
  printf("execvp fail!");
  return -1;
}

int do_exec(char **arg1, int oprion){

  int fd, pid;

  if( (pid = fork()) == 0){ 
    execvp(*arg1, arg1);
  }else if(0 < pid){
    wait(NULL);
  }else if(pid < 0){
    return -1;
  }

  return 0;
}

int do_pipe(char **arg1, char **arg2, int option){

  int fd[2], pid, pid2, n, status;
  char line[MAX];

  if( (pid = fork()) < 0 )
    printf("pipe fork error! \n");
  else if(pid == 0){

    if(pipe(fd) < 0)
      printf("pipe error! \n");

    printf("in pipe, %d, %d \n", fd[0], fd[1]);

    if( (pid2 = fork()) < 0 )
      printf("fork2 error! \n");
    else if( pid2 == 0 ){
      printf("pid0 %s \n", *arg1);

      if(dup2(fd[1], STDOUT_FILENO) < 0)
        printf("dup2 error! \n");

      close(fd[0]);
      close(fd[1]);

      execvp(*arg1, arg1);
      printf("error! \n");

    }else if( 0 < pid2 ){
      //printf("pid1 %s \n", *arg2);

      if(dup2(fd[0], STDIN_FILENO) < 0)
        printf("dup2 error! \n");
      close(fd[0]);
      close(fd[1]);
      execvp(*arg2, arg2);

    }
  }else if(0 < pid){
    wait(&status);
  }


}

int do_redirection(char **arg1, char **arg2, int option){

  int fd, pid;

  //printf("file name is : %s \n", *arg2);

  if( (pid = fork()) == 0){   
    if(option == RED_R){
      if( (fd = open(*arg2, O_RDWR | O_CREAT | O_TRUNC , 0644)) < 0 ){
        printf("open erroe! \n");
      }
      dup2(fd, 1);
    }else if(option == RED_L){
      if( (fd = open(*arg2, O_RDWR | O_CREAT , 0644)) < 0 ){
        printf("open erroe! \n");
      }
      dup2(fd, 0);
    }
    execvp(*arg1, arg1);
  }else if(0 < pid){
    wait(NULL);
  }else if(pid < 0){
    return -1;
  }

  return 0;

}

int do_chdir(char **artg1, char **arg2, int option){
  if( chdir(*arg2) < 0) 
    return -1;
  else
    return 0;
}

int main(){
  int i;
  char buf[MAX];
  char *arg1[MAX];
  char *arg2[MAX];
  char shell_str[MAX] = "[earotis]$ ";
  int red = -1;

  for(i=0; i<10; i++){
    memset(buf, 0, MAX);
    memset(arg1, 0, MAX);
    memset(arg2, 0, MAX);

    write(1, shell_str, MAX);
    read(0, buf, MAX);

    red = get_token(buf, arg1, arg2);

    switch (red){
      case 0 :
        //printf("diff %d \n", red);
        do_exec(arg1, red);
        break;
      case RED_R : 
        //printf("RED_R %d \n", red);
        do_redirection(arg1, arg2, red);
        break;
      case RED_L : 
        //printf("RED_L %d \n", red);
        do_redirection(arg1, arg2, red);
        break;
      case PIPE : 
        //printf("PIPE %d \n", red);
        do_pipe(arg1, arg2, red);
        break;
      case HELP :
        printf("***************************************\n");
        printf("** earotis@gmail.com \n");
        printf("** not provide 2 over options redirection \n");
        printf("***************************************\n");
        break;
      case CD :
        do_chdir(arg1, arg2, red);
        break;
    }

  }
}

