#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_SIZE 100000

int get_address(char *string){
	string[0] = getchar() ;
	while(string[0] == ' ') string[0] = getchar() ;
	if(string[0] == '\n') return 0 ;
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
	return 1 ;
}

// doesn't support \n \\ \* \"
void get_text(char *string){
	string[0] = getchar() ;
	while(string[0] == ' ') string[0] = getchar() ;
	if(string[0] == '"' ){
		for(int i = 0 ; ;i++){
			string[i] = getchar() ;
			if(string[i] == '"' && string[i-1] != '\\' ){
				string[i] = '\0' ;
				break ;
			}
		}
	}
	else{
		for(int i = 1 ; ; i++){
			string[i] = getchar() ;
			if(string[i] == ' ' ){
				string[i] = '\0' ;
				break ;
			}
		}
	}
}

void readrest( char *str , FILE *fob){
	int str_size = 0 ;
	for(char c = fgetc(fob);c!=EOF; c = fgetc(fob)){
		str[str_size] = c ;
		str_size++ ;
	}
	str[str_size] = '\0' ;
}

void readto(char *str , FILE *fob , int line_pos , int char_pos ){
	char c ;
	int str_size = 0 ;
	for(int line = 0 ; line < line_pos-1 ;){
		c = fgetc(fob) ;
		str[str_size] = c ;
		str_size++ ;
		if(c == '\n') line++ ;
	}
	for(int i = 0 ; i < char_pos ; i++){
		c = fgetc(fob) ;
		str[str_size] = c ;
		str_size++ ;
	}
	str[str_size] = '\0' ;
}
