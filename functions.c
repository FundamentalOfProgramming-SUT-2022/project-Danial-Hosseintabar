#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "input.c"

#define MAX_FILENAME_SIZE 1000
#define MAX_SIZE 100000

char *name_of_file(char *address){
	int u = 0 ;
	for(int i = 0 ; i < strlen(address) ; i++){
		if(address[i] == '/') u = i ;
	}
	return address+u+1 ;
}

void strdel(char *str , int index){
	for(int i = index ; i < strlen(str) ; i++){
		str[i] = str[i+1] ;
	}
}

void copytoclipboard( char *string ){
	const char* output = string ;
	const size_t len = strlen(output) + 1;
	HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
	memcpy(GlobalLock(hMem), output, len);
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
}

void insrtchtostr(char *str , char ch , int index ){
	char *nstr = (char*) malloc( MAX_SIZE * sizeof(char) ) ;
	for(int i = 0 ; i < index ; i++){
		nstr[i] = str[i] ;
	}
	nstr[index] = ch ;
	for(int i = index ; i < strlen(str) ; i++){
		nstr[i+1] = str[i] ;
	}
	nstr[strlen(str)+1] = '\0' ;
	strcpy(str , nstr) ;
}	

void insrtstrtostr( char *a , char*b , int index ){
	char *nstr = (char*) malloc( MAX_SIZE * sizeof(char) ) ;
	for(int i = 0 ; i < index ;i++){
		nstr[i] = b[i] ;
	}
	for(int i = 0 ; i< strlen(a) ;i++){
		nstr[ index + i ] = a[i] ;
	}
	for(int i = index ; i < strlen(b) ; i++){
		nstr[i+strlen(a)] = b[i] ;
	}
	nstr[strlen(a) + strlen(b) ] = '\0' ;
	strcpy(b,nstr) ;
}

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
		if(prefix[i] == '\\' && prefix[i+1] == '*' ){
			if( string[i] != '*' ) return 0 ;
			return is_prefix( prefix+i+2 , string+i+1 ) ;
		}
		else if(prefix[i] != '*'){
			if(prefix[i] != string[i]) return 0 ;
		}
		else if(prefix[i] == '*'){
			int ret = 0 ;
			for(int j = 1  ; (string[i+j-1] != EOF) && (string[i+j-1] != '\n') && (string[i+j-1] != ' ') && (i+j <= strlen(string)) ; j++){
				ret |= is_prefix(prefix+(i+1) , string+(i+j) ) ;
			} 
			return ret ;
		}

	}
	return 1 ;
}

int check_path(char *add){
	int n = strlen(add) ;
	int u ;
	for(int i = 0 ; i < n ; i++) if(add[i] == '/') u = i ;
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
	char *snapshot_file_name = (char*)malloc(1000*sizeof(char)) ;
	strcpy(snapshot_file_name , "ss__") ;
	snapshot_file_name[2] = 'A'+(snapshot_count[file_index]-1) ;
	file_name = name_of_file(file_name) ;
	strcat(snapshot_file_name , file_name) ;
	//printf("writin to %s\n" , snapshot_file_name) ;
	FILE *fob = fopen(snapshot_file_name,"w+") ;
	fprintf(fob , "%s" , text) ;
	//strcpy(snapshot[file_index][snapshot_count[file_index]-1] , text) ;
	fclose(fob) ;
	return 1 ;
}

void arman_save(char *text){
	FILE *fob = fopen("tmp_file.txt" , "w") ;
	fprintf(fob , "%s" , text) ;
	fclose(fob) ;
}

void stroneline(char *text){
	int n = strlen(text) ;
	char *newtext = (char *)malloc(MAX_SIZE * sizeof(char)) ;
	int newtext_size = 0 ;
	for(int i = 0 ; i < n ; i++){
		if(text[i] != '\n'){
			newtext[newtext_size] = text[i] ;
			newtext_size++ ;
		}
	}
	newtext[newtext_size] = '\0' ;
	strcpy(text , newtext) ;
}

void make_dir(char *add){
	char *text = (char*) malloc(MAX_SIZE * sizeof(char)) ;
	strcpy(text , add) ;
	for(int i = 0 ; i < strlen(text) ; i++){
		if( text[i] == '/' ){
			text[i] = '\0';
			mkdir(text) ;
			text[i] = '/' ;
		}
	}
}
