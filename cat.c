#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cat(char *command){
	char file_name[MAX_SIZE] ;
	char opt[MAX_SIZE] ;

	//GET INPUT
	if(strcmp(command , "cat")) return 0 ;
	scanf("%s" , opt ) ;
	if(strcmp(opt , "-file")) return 0 ;
	getchar() ;
	get_address(file_name) ;
	FILE *fob = fopen(file_name,"r") ;
	char line[MAX_SIZE] ;
	fgets(line , MAX_SIZE , fob) ;

}
