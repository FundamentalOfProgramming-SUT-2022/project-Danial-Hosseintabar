#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "createfile.c"
#include "input.c"
#include "insertstr.c"
#include "cat.c"

#define MAX_SIZE 10000

int check_exit(char *command){
	return !strcmp(command , "exit" ) ;
}

void get_text( char string[MAX_SIZE] ){
	scanf("%s" , string) ;
}

int main(){

	printf("FOP 2022 PROJECT : Danial Hosseintabar\n") ;

	while(1){
		int command_run = 0 ;
		char command[MAX_SIZE] ;

		scanf("%s" , command) ;

		// USE ARRAY !!!

		if( check_exit(command) )
			break ;

		if( check_createfile(command) ){
			command_run = 1 ;
		}
		if( check_insertstr(command) ){
			command_run = 1 ;
		}

		if(command_run == 0){
			printf("Invalid input!\n") ;
		}

	}

	printf("Press Enter to exit ...") ;
	getchar() ;
}


