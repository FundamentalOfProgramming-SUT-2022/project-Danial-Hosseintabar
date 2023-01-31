#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "input.c"

#define MAX_FILENAME_SIZE 256
#define MAX_SIZE 100000


void dirtree_search(char *add , int depth , int N , FILE *fob){
	for(int i = 0 ; i < N-1-depth ; i++){printf("    "); fprintf(fob , "    " ) ;}
	int p = 0;
	for(int i = strlen(add) ; i >= 0 ; i--){
		if(add[i] == '/'){
			p = i+1 ;
			break ;
		}
	}
	printf("%s" , add+p);
	fprintf(fob , "%s" , add+p) ;
	int ret = 0 ;
  	struct dirent *files;
 	DIR *dir = opendir(add);
   	if (dir == NULL){
			printf("\n");
			fprintf(fob , "\n") ;
      		return ;
   	}
	printf(" :\n") ;
	fprintf(fob , " :\n") ;
	int u = 0 ;
   	while ((files = readdir(dir)) != NULL){
		if( u < 2 ){u++ ; continue ;}
   		ret++ ;
		char *add_temp = (char*)malloc(1000*sizeof(char)) ;
		strcpy(add_temp , add) ;
		strcat(add_temp , "/" ) ;
		strcat(add_temp , files->d_name);
		if(depth > 0){
			dirtree_search(add_temp , depth-1 , N , fob) ;
		}
		else{	
			for(int i = 0 ; i < N-depth-1 ; i++){
				printf("     ") ;
				fprintf(fob , "    ") ;
			}
			printf("    %s\n" , files->d_name) ;
			fprintf(fob , "    %s\n" , files->d_name) ;
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

int check_arman(){
	char c = getchar() ;
	while( c == ' ' ) c = getchar() ;
	if(c == '\n'){
		return 0 ;
	}
	else if( c == '=' && getchar() == 'D'){
		return 1 ;
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

void clear_file(char* file_name){
	FILE *fob = fopen(file_name , "w") ;
	fclose(fob) ;
}

int take_snapshot(char *file_name , int snapshot_count[] , char*snapshot_fn[] , int* file_count ){
	
	FILE *fp = fopen(file_name , "r") ;
	char *text = (char*) malloc( MAX_SIZE * sizeof(char) ) ;
	readrest(text , fp) ;
	fclose(fp) ;

	int file_index = *file_count ;
	for(int i = 0 ; i < *file_count ; i++){
		if(strcmp(file_name , snapshot_fn[i])) continue ;
		file_index = i ;
		break ;
	}
	if(file_index == *file_count) strcpy( snapshot_fn[*file_count] , file_name ) ;
	*file_count += ( file_index == *file_count ? 1 : 0 ) ;
	snapshot_count[file_index]++ ;
	
	char *snapshot_file_name = (char*)malloc(256*sizeof(char)) ;
	strcpy(snapshot_file_name , "ss_") ;
	snapshot_file_name[2] = 'A'+(snapshot_count[file_index]-1) ;
	strcat(snapshot_file_name , file_name) ;
	FILE *fob = fopen(snapshot_file_name,"w+") ;
	fprintf(fob , "%s" , text) ;
	//strcpy(snapshot[file_index][snapshot_count[file_index]-1] , text) ;
	fclose(fob) ;
	return 1 ;
}
