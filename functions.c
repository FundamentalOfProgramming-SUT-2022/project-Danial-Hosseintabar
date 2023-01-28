#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//  character \* \n not supported yet
int is_prefix(char *prefix , char *string){
	int n = strlen(prefix) ;
	int N = strlen(string) ;
	if(n>N) return 0 ;
	for(int i = 0 ; i < n ; i++){
		if(prefix[i] != '*'){
			if(prefix[i] != string[i]) return 0 ;
		}
		if(prefix[i] == '*'){
			int ret = 0 ;
			// ret = is_prefix(prefix+(i+1),string+(i+1)) | is_prefix(prefix+(i+1),string+(i+2)) | is_prefix(prefix+(i+1),string+(i+3)) | ...
			for(int j = 1 ; i+j <= strlen(string) ; j++){
				ret |= is_prefix(prefix+(i+1) , string+(i+j) ) ;
			} 
			return ret ;
		}

	}
	return 1 ;
}