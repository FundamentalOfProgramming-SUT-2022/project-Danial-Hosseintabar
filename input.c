#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
