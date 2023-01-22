#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int check_insertstr(char *command){
	int line_pos ;
	int char_pos ;
	char file_name[MAX_SIZE] ;
	char opt[MAX_SIZE] ;
	char string[MAX_SIZE] ;

	// no file found error ?

	// GETTING INPUT
	if(strcmp(command , "insertstr")) return 0 ;
	scanf("%s" , opt ) ;
	if(strcmp(opt , "-file")) return 0 ;
	getchar() ;
	get_address(file_name) ;
	scanf("%s" , opt ) ;
	if(strcmp(opt , "-str")) return 0 ;
	getchar() ;
	get_address(string) ;
	scanf("%s" , opt ) ;
	if(strcmp(opt , "-pos")) return 0 ;
	getchar() ;
	if(2!=scanf("%d:%d" , &line_pos , &char_pos)) return 0 ;

	// debug : printf("file : %s , str : %s , line_pos : %d , char_pos : %d \n" , file_name , string , line_pos , char_pos ) ;

	FILE *fob = fopen(file_name , "r+" ) ;

	for(int i = 0 ; i < line_pos-1 ; i++){
		char string_tmp[MAX_SIZE] ;
		fgets(string_tmp , MAX_SIZE , fob) ;
	}

	fseek(fob , char_pos*sizeof(char) , SEEK_CUR) ;
	fprintf(fob , "%s" , string ) ;
	fclose(fob) ;

	return 1 ;

}
