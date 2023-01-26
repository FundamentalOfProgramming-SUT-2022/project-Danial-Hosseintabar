#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SIZE 100000

void get_address(char string[MAX_SIZE]){
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
