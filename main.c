#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "input.c"

#define MAX_SIZE 100000

int check_exit(char *command){
	return !strcmp(command , "exit" ) ;
}

int check_createfile(char *command){
	char opt[MAX_SIZE] ;
	char add[MAX_SIZE] ;
	if(strcmp(command , "createfile")) return 0 ;
	scanf("%s" , opt) ;
	if(strcmp("-file" , opt)) return 0 ;
	getchar() ;
	get_address(add) ;
	printf("FILE CREATED SUCCESSFULLY : ( %s )\n" , add ) ;
	FILE *fob = fopen(add , "a") ;
	fclose(fob) ;
	return 1 ;
}

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

int check_cat(char *command){
	if(strcmp(command,"cat")) return 0 ;
	char opt[MAX_SIZE] ;
	char add[MAX_SIZE] ;
	scanf("%s" , opt);
	if(strcmp(opt,"-file")) return 0 ;
	getchar() ;
	get_address(add) ;

	FILE *fob = fopen(add , "r") ;
	char c = fgetc(fob);
	while(c!=EOF){
		printf("%c" , c) ;
		c = fgetc(fob) ;
	}
	
	printf("\n") ;
	
	fclose(fob) ;
	return 1 ;
}

int check_removestr(char *command){
	char file_name[MAX_SIZE] ;
	char arg[MAX_SIZE] ;
	int line_pos ;
	int char_pos ;
	int size ;
	int mode ;

	if(strcmp(command , "removestr")) return 0 ;
	scanf("%s" , arg) ;
	if(strcmp(arg,"-file")) return 0 ;
	scanf("%s" , file_name);

	scanf("%s" , arg) ;
	if(strcmp(arg,"-pos")) return 0 ;
	scanf("%d:%d" , &line_pos , &char_pos ) ;

	scanf("%s" , arg) ;
	if(strcmp(arg,"-size")) return 0 ;
	scanf("%d" , &size) ;

	scanf("%s" , arg) ;
	if(!strcmp(arg , "-b")) mode = 1 ; // backward
	else mode = 0 ; // forward

	//forward
	char text_pre[MAX_SIZE] ;
	char text_post[MAX_SIZE] ;
	FILE *fob = fopen(file_name , "r") ;
	readto(text_pre , fob , line_pos , char_pos) ;
	//fseek(fob , size , SEEK_CUR) ;
	readrest(text_post , fob) ;
	fclose(fob) ;
	
	if(mode){
		for(int i = 0 ; i < size ; i++){
			text_pre[strlen(text_pre)-1] = '\0' ;
		}
	}

	fob = fopen(file_name , "w") ;
	fprintf(fob , "%s%s" , text_pre , text_post+size*(1-mode) ) ;
	fclose(fob) ;
	return 1 ;


}

int check_copystr(char *command){
	char arg[MAX_SIZE] ;
	char file_name[MAX_SIZE] ;
	int size = 0 ;
	int str_size = 0 ;
	int line_pos ;
	int char_pos ;
	int mode ;

	if(strcmp(command,"copystr")) return 0 ;
	
	scanf("%s" , arg) ;
	if(strcmp(arg,"-file")) return 0 ;
	getchar() ;
	get_address(file_name) ;
	
	scanf("%s",arg) ;
	if(strcmp("-pos",arg)) return 0 ;
	scanf("%d:%d" , &line_pos , &char_pos) ;
	
	scanf("%s",arg) ;
	if(strcmp("-size",arg)) return 0 ;
	scanf("%d" , &size) ;

	scanf("%s" , arg) ;
	if(!strcmp(arg,"-b")) mode = 1 ;
	else mode = 0 ;
	
	char trash[MAX_SIZE] ; 
	char str[MAX_SIZE] ;

	FILE *fob = fopen(file_name , "r") ;
	readto(trash , fob , line_pos , char_pos) ;
	fseek(fob , -size*mode , SEEK_CUR) ;
	for(int i = 0 ; i < size ; i++){
		char c = fgetc(fob) ;
		str[str_size] = c ;
		str_size++ ;
	}
	str[str_size] = '\0' ;
	fclose(fob) ;
	
	// COPYING THE TEXT TO CLIPBOARD
	const char* output = str ;
	const size_t len = strlen(output) + 1;
	HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
	memcpy(GlobalLock(hMem), output, len);
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();

	//printf("following text is copied :\n%s\n",str) ;
	
	return 1 ;
}

int main(){

	printf("FOP 2022 PROJECT : Danial Hosseintabar\n") ;

	while(1){
		int command_run = 0 ;
		char command[MAX_SIZE] ;

		scanf("%s" , command) ;

		if(!strcmp("exit",command)) return 0 ;
		
		int (*check_function[])(char*) = { check_createfile , check_insertstr , check_cat , check_removestr , check_copystr } ;
		
		for(int i = 0 ; i < 5 ; i++){
			if( (*check_function[i])(command) ){
				command_run = 1 ;
			}
		}

		if(command_run == 0){
			char trash[MAX_SIZE] ;
			gets(trash) ;
			printf("Invalid input!\n") ;
		}

	}
}
