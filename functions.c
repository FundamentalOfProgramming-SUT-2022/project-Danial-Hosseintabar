#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "input.c"

#define MAX_FILENAME_SIZE 256
#define MAX_SIZE 100000

struct snapshot{
	char file_name[MAX_FILENAME_SIZE] ;
	char **snapshot ;
};

void reset_snapshot(struct snapshot* snapshot_ptr){
	(*snapshot_ptr).file_name[0] = '\0' ;
	(*snapshot_ptr).snapshot = NULL ;
}

void dirtree_search(char *add , int depth , int N){
	for(int i = 0 ; i < N-1-depth ; i++) printf("    ");
	int p = 0;
	for(int i = strlen(add) ; i >= 0 ; i--){
		if(add[i] == '/'){
			p = i+1 ;
			break ;
		}
	}
	printf("%s" , add+p);
	int ret = 0 ;
  	struct dirent *files;
 	DIR *dir = opendir(add);
   	if (dir == NULL){
			printf("\n");
      		return ;
   	}
	printf(" :\n") ;
	int u = 0 ;
   	while ((files = readdir(dir)) != NULL){
		if( u < 2 ){u++ ; continue ;}
   		ret++ ;
		char *add_temp = (char*)malloc(1000*sizeof(char)) ;
		strcpy(add_temp , add) ;
		strcat(add_temp , "/" ) ;
		strcat(add_temp , files->d_name);
		if(depth > 0){
			dirtree_search(add_temp , depth-1 , N) ;
		}
		else{	
			for(int i = 0 ; i < N-depth-1 ; i++){
				printf("     ") ;
			}
			printf("    %s\n" , files->d_name) ;
		} 
		//printf("%s\n", files->d_name);
   	}
   	closedir(dir);
   	return ;
}

//  character \* \" \n not supported yet
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

int cmp( const void* a ,const void *b){
	return *((int*)a) > *((int*)b) ;
}

void print_tab(int x , FILE *fob){
	for(int i = 0 ;i  < x ;i++) fprintf(fob , "    ") ;
}

void take_snapshot(FILE *fob , char* file_name , struct snapshot file_history[] ){
	fseek(fob , 0 , SEEK_END) ;
	char *text = (char*)malloc(ftell(fob) * sizeof(char)) ;
	fseek(fob , 0 , SEEK_SET) ;
	readrest(text , fob) ;
	int index = -1 ;
	int size = 0 ;
	for(int i = 0 ; file_history[i].file_name[0] != '\0' ; i++){
		if(!strcmp(file_history[i].file_name , file_name)){
			index = i ; 
			break ;
		}
		size++ ;
	}
	if(index==-1){
		memcpy(file_history[size].file_name,file_name,strlen(file_name)+1);
		file_history[size].snapshot[0] = NULL;
		file_history[size+1].file_name[0] = '\0' ;
		index = size ;
	}
	for(int i = 0 ;  ;i++){
		if(file_history[index].snapshot[i] == NULL){
			file_history[index].snapshot[i] = text ;
			file_history[index].snapshot[i+1] = NULL ;
			break ;
		}
	}
}

int getwords(char *text , char words[][50] ){
	int index = 0 ;
	int wordcount = 0 ;
	while( 1 ){
		while( text[index] == ' ' ) index++ ;
		if(text[index] == EOF || text[index] == '\0' || text[index] == '\n') break ;
		int i = 0 ; 
		while(text[index] != ' ' && text[index] != '\0' && text[index] != '\n'){
			words[wordcount][i] = text[index] ;
			i++ ;
			index++;
		}
		words[wordcount][i] = '\0' ;
		wordcount++ ;
	}
	return wordcount ;
}