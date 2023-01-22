#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int check_createfile(char *command){
	char opt[MAX_SIZE] ;
	char add[MAX_SIZE] ;
	if(!streq(command , "createfile")) return 0 ;
	scanf("%s" , opt) ;
	if(!streq("-file" , opt)) return 0 ;
	getchar() ;
	get_address(add) ;
	printf("FILE CREATED SUCCESSFULLY : ( %s )\n" , add ) ;
	FILE *fob = fopen(add , "a") ;
	fclose(fob) ;
	return 1 ;
}
