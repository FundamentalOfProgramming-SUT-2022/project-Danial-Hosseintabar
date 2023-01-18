#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 50

int check_exit(char *command){
	return !memcmp(command , "exit" , 4) && command[4] == NULL ;
}

void get_double_quote(char string[MAX_SIZE]){
	scanf("%c" , string ) ;
	if(string[0] == '"'){
		for(int i = 0 ; ; i++ ){
			string[i] = getchar() ;
			if(string[i] == '"'){
				string[i] = '\0' ;
				break ;
			}
		}
	}
	else{	
		scanf("%s" , string+1) ;
	}
	return &string[0] ;
}

int check_createfile(char *command){
	char opt[MAX_SIZE] ;
	char add[MAX_SIZE] ;
	if(memcmp(command , "createfile" , 10)) return 0 ;
	scanf("%s" , opt) ;
	if(memcmp(opt , "-file" , 5)) return 0 ;
	getchar() ;
	get_double_quote(add) ;
	printf("FILE CREATED SUCCESSFULLY : ( %s )\n" , add ) ;
	FILE *fob = fopen(add , "a") ;
	fclose(fob) ;
	return 1 ;
}

int main(){
	
	printf("FOP 2022 PROJECT : Danial Hosseintabar\n") ;
	
	while(1){
		int command_run = 0 ;	
		char command[MAX_SIZE] ;
		
		scanf("%s" , command) ;	
		
		if( check_exit(command) )
			break ;
		
		if( check_createfile(command) ){
			command_run = 1 ;
		}

		if(command_run == 0){
			printf("Invalid input!\n") ;
		}

	}
	
	printf("Press Enter to exit ...") ;
	getchar() ;
}












