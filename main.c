#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "conio2.h"
#include <unistd.h>
#include <minwindef.h>
// #include <happiness.h>
#include "conio.c"
#include "functions.c"

#define SCR_HEIGHT 30
#define SCR_WIDTH 100
#define TEXT_START 7
#define SS_MAX 100
#define FN_MAX 1000

// GLOBAL ARRAYS FOR UNDO FUNCTIONING
int snapshot_count[SS_MAX] ;
char *snapshot_fn[SS_MAX] ;
int file_count = 0 ;

// GLOBAL VARIABLES FOR ARMAN COMMAND
int arman_mode = 0 ;
char arman_str[MAX_SIZE] ;

// SOME GUI-RELATED GLOBAL VARIABLES
enum mode_type{ NORMAL , VISUAL , INSERT } ;
enum mode_type MODE = NORMAL ;
FILE *opened_file = NULL ; 
char *opened_text ;
char *opened_file_name ;
int opened_text_line_count = 0 ;
int opened_text_line_length[FN_MAX]  = {0} ;
int FIRST_LINE_NUM = 1 ;
int cursor_x = 0;
int cursor_y = TEXT_START;
int EXIT = 0 ;
int oti = 0 ; // opened_text_index
int otil = 0 ; // opened_text_index (in current line)
int selected_start = -1 ;
int selected_end = -1 ;
int selected = 0 ;
int saved_status = 1 ;
int find_count = 0 ;
int find_highlight[FN_MAX][2];
int find_highlight_status = 0 ;
int auto_indent_opened_file = 0 ;
int undo_opened_file = 0 ;
int cur_fh_i = 0 ;
int find_in_of = 0 ;
int mustsave = 0 ;

int find_pos(int index , int *x , int *y){
	(*x) = 0 ;
	(*y) = TEXT_START ;
	for(int i = 0 ; i < index ; i++){
		if(opened_text[i] == '\n'){
			(*y) = TEXT_START ;
			(*x)++ ;
		}
		else{
			(*y)++ ;
		}
	}
}

void save_opened_file(){
	take_snapshot(opened_file_name , snapshot_count , snapshot_fn , &file_count) ;
	saved_status = 1 ;
	fclose(opened_file) ;
	fopen(opened_file_name , "w") ;
	fprintf(opened_file , "%s" , opened_text) ;
	fclose(opened_file) ;
	fopen(opened_file_name , "r+") ;
}

void update_opened_text(){
	fseek(opened_file , 0 , SEEK_SET) ;
	readrest(opened_text , opened_file) ;
}

void RESET_SS_VARS(){
	for(int i = 0 ; i < SS_MAX ; i++){
		snapshot_count[i] = 0 ;
		snapshot_fn[i] = (char*)malloc(FN_MAX*sizeof(char)) ;
		snapshot_fn[i][0] = '\0' ;
	}
}

int check_createfile(char *command){
	char opt[MAX_SIZE] ;
	char add[MAX_SIZE] ;
	if(strcmp(command , "createfile")) return 0 ;
	scanf("%s" , opt) ;
	if(strcmp("-file" , opt)) return 0 ;
	getchar() ;
	get_address(add) ;
	if(fopen(add , "r") == NULL){
		make_dir(add) ;
		FILE *fob = fopen(add , "a") ;
		fclose(fob) ;
		return 1 ;
	}
	else{
		printf("FILE ALREADY EXISTS!\n") ;
		if(check_arman()){
			arman_mode = 1 ;
			arman_save("FILE ALREADY EXISTS!\n") ;
		}
		return 1 ;
	}
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
	get_address(file_name) ;
	

	if(!arman_mode){get_str_arg(string) ;}
	else{
		FILE *tmp_file = fopen("tmp_file.txt" , "r") ;
		fseek(tmp_file , 0 , SEEK_SET) ;
		readrest(string , tmp_file) ;
		fclose(tmp_file) ;
		arman_mode = 0 ;
	}
	scanf("%s" , opt ) ;
	if(strcmp(opt , "-pos")) return 0 ;
	if(2!=scanf("%d:%d" , &line_pos , &char_pos)) return 0 ;
	
	// debug : printf("file : %s , str : %s , line_pos : %d , char_pos : %d \n" , file_name , string , line_pos , char_pos ) ;

	if(fopen(file_name , "r") == NULL){
		printf("WRONG ADDRESS!\n") ;
		if(check_arman()){
			arman_save("WRONG ADDRESS!\n") ;
			arman_mode = 1; 
		}
		return 1 ;
	}
	// TAKING SNAPSHOT

	take_snapshot(file_name , snapshot_count , snapshot_fn , &file_count) ;
	
	// INSERTING
	FILE *fob = fopen(file_name , "r" ) ;
	char *text_pre = (char*) malloc(MAX_SIZE*sizeof(char)) ;
	char *text_post = (char*) malloc(MAX_SIZE*sizeof(char)) ;
	readto(text_pre , fob , line_pos , char_pos) ;
	readrest(text_post , fob) ;
	fclose(fob) ;

	fob = fopen(file_name , "w") ;
	fprintf(fob , "%s%s%s" ,text_pre , string , text_post) ;
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
	if(NULL == fopen(add , "r")){
		printf("WRONG ADDRESS\n") ;
		if(check_arman()){
			arman_save("WRONG ADDRESS") ;
			arman_mode = 1 ;
		}
		return 1 ;
	}
	FILE *fob = fopen(add , "r") ;
	char *text = (char*)malloc(MAX_SIZE*sizeof(char)) ;
	readrest(text , fob) ;
	printf("%s\n" , text) ;
	fclose(fob) ;

	if(check_arman()){
		arman_mode = 1 ;
		arman_save(text) ;
	}

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

	if(NULL == fopen(file_name , "r") ){
		printf("WRONG ADDRESS!\n") ;
		if(check_arman()){
			arman_save("WRONG ADDRESS!") ;
			arman_mode = 1 ;
		}
		return 1 ;
	}

	take_snapshot(file_name , snapshot_count , snapshot_fn , &file_count) ;

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

	if(NULL == fopen(file_name , "r") ){
		printf("WRONG ADDRESS!\n") ;
		if(check_arman()){
			arman_save("WRONG ADDRESS!") ;
			arman_mode = 1 ;
		}
		return 1 ;
	}

	char trash[MAX_SIZE] ; 
	char *str = (char*) malloc( 1 + size * sizeof(char)) ;

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

int check_cutstr(char *command){
	char arg[MAX_SIZE] ;
	char file_name[MAX_SIZE] ;
	int size = 0 ;
	int str_size = 0 ;
	int line_pos ;
	int char_pos ;
	int mode ;

	if(strcmp(command,"cutstr")) return 0 ;
	
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

	if(NULL == fopen(file_name , "r") ){
		printf("WRONG ADDRESS!\n") ;
		if(check_arman()){
			arman_save("WRONG ADDRESS!") ;
			arman_mode = 1 ;
		}
		return 1 ;
	}

	take_snapshot(file_name , snapshot_count , snapshot_fn , &file_count) ;

	char *string = (char*) malloc( 1 + size*sizeof(char) ) ;
	FILE *fob = fopen(file_name , "r+") ;
	char text_pre[MAX_SIZE] ;
	char text_post[MAX_SIZE] ;
	readto(text_pre , fob , line_pos , char_pos ) ;
	readrest(text_post , fob) ;
	fclose(fob) ;
	fopen(file_name , "w") ;
	if(mode){
		for(int i = strlen(text_pre)-size ; i < strlen(text_pre) ; i++ ){
			string[i + size -strlen(text_pre) ] = text_pre[i] ;
			text_pre[i] = '\0' ;
		}
		string[size] = '\0' ;
	}
	else{
		for(int i = 0 ; i < size ; i++)
			string[i] = text_post[i] ;
		string[size] = '\0' ;
	}
	fprintf(fob , "%s%s" , text_pre , text_post + (1-mode)*size*sizeof(char) ) ;
	fclose(fob) ;
	// COPYING THE TEXT TO CLIPBOARD
	const char* output = string ;
	const size_t len = strlen(output) + 1;
	HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
	memcpy(GlobalLock(hMem), output, len);
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
	//printf("the following text is cut :\n%s\n",string) ;
	return 1 ;
}

int check_pastestr(char *command){
	int line_pos ;
	int char_pos ;
	char arg[MAX_SIZE] ;
	char file_name[MAX_SIZE] ;
	char string[MAX_SIZE] ;

	if(strcmp(command,"pastestr")) return 0 ;
	scanf("%s" , arg) ;
	if(strcmp("-file",arg)) return 0 ;
	getchar() ;
	get_address(file_name) ;
	scanf("%s" , arg) ;
	if(strcmp("-pos",arg)) return 0 ;
	scanf("%d:%d" , &line_pos , &char_pos) ;

	if(NULL == fopen(file_name , "r") ){
		printf("WRONG ADDRESS!\n") ;
		if(check_arman()){
			arman_save("WRONG ADDRESS!") ;
			arman_mode = 1 ;
		}
		return 1 ;
	}

	take_snapshot(file_name , snapshot_count , snapshot_fn , &file_count) ;

	OpenClipboard(0) ;
	HANDLE in = GetClipboardData(CF_TEXT) ;
	strcpy(string , (char*)in) ;
	CloseClipboard() ;

	char text_pre[MAX_SIZE] ;
	char text_post[MAX_SIZE] ;
	FILE *fob = fopen(file_name , "r+") ;
	readto(text_pre , fob , line_pos , char_pos) ;
	readrest(text_post , fob) ;
	fclose(fob) ;
	fob = fopen(file_name , "w") ;
	fprintf(fob , "%s%s%s",text_pre, string , text_post) ;
	fclose(fob) ;
	
	return 1 ;
}

int check_find(char *command){
	if( !find_in_of && strcmp(command , "find")) return 0 ;
	char string[MAX_SIZE] ;
	char arg[MAX_SIZE] ;
	char file_name[MAX_SIZE] ;
	int at = -1 ;
	int mask = 0 ; // mask = count , at , byword , all
	char c ;
	if( !find_in_of ){
		if(!arman_mode) get_str_arg(string) ;
		else{
			FILE *tmp_file = fopen("tmp_file.txt","r") ;
			fseek(tmp_file , 0 , SEEK_SET) ;
			readrest(string,tmp_file) ;
			arman_mode = 0 ;
			fclose(tmp_file) ;
		}

		scanf("%s" , arg) ;
		if(strcmp(arg , "-file")) return 0 ;
		get_address(file_name) ;
		c = getchar() ;
		
		while(c!='\n'){
			c = getchar() ;
			while(c==' ') c = getchar() ;
			if(c == '\n') break ;
			scanf("%s" , arg) ;
			if(!strcmp(arg,"count")){
				mask += (1<<3) ;
			}
			if(!strcmp(arg,"at")){
				mask += (1<<2) ;
				scanf("%d" , &at) ;
			}
			if(!strcmp(arg,"byword")){
				mask += (1<<1) ;
			}
			if(!strcmp(arg,"all")){
				mask += 1 ;
			}
			if(!strcmp(arg , "D")){
				arman_mode = 1 ;
				break ;
			}
		}
		
		if(NULL == fopen(file_name , "r") ){
			clrscr() ;
			printf("%s" , file_name) ;
			getch() ;
			printf("WRONG ADDRESS!\n") ;
			if(check_arman()){
				arman_save("WRONG ADDRESS!") ;
				arman_mode = 1 ;
			}
			return 1 ;
		}
	}
	else{
		strcpy(string , command) ;
		strcpy(file_name , opened_file_name) ;
		find_in_of = 0 ;
		mask = 0 ;
	}
	char *text = (char*) malloc(MAX_SIZE * sizeof(char) ) ;
	strcpy(text , opened_text) ;
	
	int ans[MAX_SIZE] ;
	for(int i = 0 ; i < MAX_SIZE ; i++) ans[i] = -1 ;
	int count = 0 ;
	int ok = 0 ;
	if(1&(mask>>1)){
		int u = 0 ;
		for(int i = 0 ; i < strlen(text) ; i++){
			if(text[i] == ' ' || text[i] == '\n'){
				u++;
				while(text[i] == ' ' || text[i] == '\n') i++ ;
			}
			if( is_prefix(string , text+i) ){
				ans[count] = u ;
				count++ ;
			}
		}
	}
	else{
		for(int i = 0 ; i < strlen(text) ; i++){
			if( is_prefix(string , text+i) ){
				ans[count] = i ;
				count++ ;
			}
		}
	}
	find_count = count ;
	
	FILE *tmp_file = fopen("tmp_file.txt" , "w") ;
	if(!count){
		printf("-1\n") ;
		fprintf(tmp_file , "-1") ;	
		return 1 ;
	}

	if(mask >> 3){
		printf("%d\n" , count);
		fprintf(tmp_file , "%d" , count) ;
	}
	if((mask >> 2)&1){
		printf("%d\n" , ans[at-1]) ;
		fprintf(tmp_file , "%d" , ans[at-1]) ;
	}
	if(mask&1){
		for(int i = 0;ans[i]!=-1;i++){
			printf("%d", ans[i] ) ;
			fprintf(tmp_file , "%d" , ans[i]) ;
			if(ans[i+1]!=-1){
				fprintf(tmp_file , ",") ;
				printf(",") ;
			}
			else{
				printf("\n") ;
			}
		}
	}
	if(mask == 0){
		fprintf(tmp_file , "%d" , ans[0]) ;
		printf("%d\n" , ans[0]) ;
	}

	for(int i = 0 ; i < count ; i++){
		find_highlight[i][0] = ans[i] ;
		find_highlight[i][1] = ans[i] + strlen(string) -1; 
	}
	find_highlight_status = 1 ;
	oti = ans[0] ;
	find_pos( oti , &cursor_x , &cursor_y) ;
	otil = cursor_y - TEXT_START ;
	fclose(tmp_file) ;
	cur_fh_i = 1 ;
	return 1 ;
}

void calculate_text(){
	opened_text_line_count = 0 ;
	for(int i = 0 , u = 0 ; i <= strlen(opened_text) ; i++){
		if( opened_text[i] == '\n' || opened_text[i] == '\0' ){
			opened_text_line_length[opened_text_line_count] = u+1 ;
			opened_text_line_count++ ;
			u = 0 ;
		}
		else{
			u++ ;
		}
	}
}

int check_replace(char *command){
	if(strcmp(command , "replace")) return 0 ;
	char arg[MAX_SIZE] ;
	char string1[MAX_SIZE] ;
	char string2[MAX_SIZE] ;
	char file_name[MAX_SIZE] ;
	if(!arman_mode){
		scanf("%s" , arg) ;
		if(strcmp(arg,"-str1")) return 0 ;
		get_text(string1) ;
	}
	else{
		FILE *tmp_file = fopen("tmp_file" , "r") ;
		fseek(tmp_file,0,SEEK_SET) ;
		readrest(string1 , tmp_file) ;
		arman_mode = 0 ;
		fclose(tmp_file) ;
	}

	scanf("%s" , arg) ;
	if(strcmp(arg,"-str2")) return 0 ;
	get_text(string2) ;

	scanf("%s" , arg) ;
	if(strcmp(arg,"-file")) return 0 ;
	get_address(file_name) ;

	take_snapshot(file_name , snapshot_count  , snapshot_fn , &file_count) ;

	int at ;
	int mode = 0 ; // 0 : normal , 1 : at , 2 : all
	char c = getchar() ;
	while(c == ' ') c = getchar() ;
	if(c != '\n'){
		scanf("%s" , arg) ;
		if(!strcmp(arg,"at")){
			scanf("%d" , &at) ;
			mode = 1 ;
		}
		else if(!strcmp(arg,"all")){
			mode = 2 ;
		}
	}

	char *text = (char*)malloc(MAX_SIZE*sizeof(char)) ;
	FILE *fob = fopen(file_name , "r");
	readrest(text , fob) ;
	fclose(fob) ;

	int ans[MAX_SIZE] ;
	for(int i = 0 ; i < MAX_SIZE ; i++) ans[i] = -1 ;
	int count = 0 ;

	// finding all matches that don't have shared charachters :
	for(int i = 0 ; i < strlen(text) ; i++){
		if(is_prefix(string1 , text+i)){
			ans[count] = i ;
			count++ ;
			i+=strlen(string1)-1;
		}
	}
	
	// replacing all the matches
	if( mode == 0 || mode == 2 ){
		fob = fopen(file_name , "w") ;
		for(int u = 0 , i = 0 ; i < strlen(text) ; i++){
			if(ans[u] == i){
				u++ ;
				fprintf(fob , "%s" , string2) ;
				i += strlen(string1) - 1 ;
				continue ;
			}
			fprintf(fob , "%c" , text[i] ) ;
		}
		fclose(fob) ;
	}

	if( mode == 1 ){
		fob = fopen(file_name , "w") ;
		for(int i = 0 ; i < strlen(text) ; i++){
			if(ans[at-1] == i){
				fprintf(fob , "%s" , string2) ;
				i += strlen(string1) - 1 ;
				continue ;
			}
			fprintf(fob , "%c" , text[i] ) ;
		}
		fclose(fob) ;
	}

	if( c!= '\n' && check_arman()){
		arman_save("Operation Successful!") ;
		arman_mode = 1 ;
	}
	printf("Operation Successful!\n") ;
	return 1 ;
}

int check_grep(char *command){
	if(strcmp(command , "grep")) return 0 ;
	int mode = 0 ; // 0 : normal , 1 : -l , 2 : -c
	char arg[MAX_SIZE] ;
	char string[MAX_SIZE] ;
	char file_name[MAX_SIZE] ;
	char *text = (char*)malloc(MAX_SIZE*sizeof(char)) ;
	
	scanf("%s" , arg) ;
	if(!strcmp(arg,"-str")) mode = 0 ;
	else if(!strcmp(arg , "-l")){
		mode = 1 ;
	}
	else if(!strcmp(arg,"-c")){
		mode = 2 ;
	}
	else if(!arman_mode) {return 0 ;}

	if( !arman_mode && mode != 0){
		scanf("%s" , arg) ;
		if(strcmp(arg , "-str")) { return 0 ;}
	}
	if(!arman_mode) get_text(string) ;
	else{
		FILE *tmp_file = fopen("tmp_file.txt" , "r") ;
		fseek(tmp_file , 0 , SEEK_SET) ;
		readrest(string , tmp_file) ;
		fclose(tmp_file) ;
	}
	if(!arman_mode){
		scanf("%s" , arg) ;
		if(strcmp("-files" , arg)) return 0 ;
	}
	if(arman_mode) arman_mode = 0 ;
	FILE *tmp_file = fopen("tmp_file.txt" , "w") ;
	int count =  0;
	while(get_address(file_name)){
		if( !strcmp(file_name,"=D") ){
			arman_mode = 1 ;
			break ;
		}
		int file_ok = 0 ;
		FILE *fob = fopen(file_name , "r") ;
		readrest(text , fob) ;
		fclose(fob) ;
		for(int u = 0 , i = 0 ; i < strlen(text) ; i++){
			if(text[i] == '\n') u = i+1 ; 
			if(is_prefix(string , text+i)){
				file_ok = 1 ;
				count++ ;
				if(mode != 2){
					printf("%s" , file_name) ;
					fprintf(tmp_file , "%s" , file_name) ;
				}
				if(mode == 0){
					printf(" : ") ;
					fprintf(tmp_file , " : ") ;
				}
				for(int j = u ; text[j] != '\n' && text[j] != EOF && j < strlen(text) ; j++){
					if(mode == 0){
						printf("%c" , text[j]) ;
						fprintf(tmp_file , "%c" , text[j]) ;
					}
					i = j ;
				}
				if(mode != 2){
					printf("\n") ;
					fprintf(tmp_file , "\n") ;
				}
			}
			if(file_ok && mode == 1) break ;
		}
		if(file_ok && mode == 1) continue ;
	}
	fclose(tmp_file);
	if(mode == 2){
		printf("%d\n" , count) ;
		fprintf(tmp_file , "%d\n" , count);
	}
	
	return 1 ;
}

int check_undo(char *command){
	if(strcmp("undo" , command)) return 0 ;
	char *text = (char *) malloc(MAX_SIZE * sizeof(char)) ;
	char arg[MAX_SIZE] ;
	int ok = 0 ;
	char *file_name = (char*) malloc(MAX_SIZE * sizeof(char)) ;
	if( !undo_opened_file ){
		char c =getchar() ;
		while( c == ' ' ) c = getchar() ;
		
		scanf("%s" , arg) ;
		if(strcmp(arg , "-file")) return 0 ;
		get_address(file_name) ;
		undo_opened_file = 0 ;
	}
	else{
		strcpy(file_name , opened_file_name) ;
	}
	FILE *fob ;
	FILE *fp ;
	for(int i = 0 ; i < file_count ; i++){
		if(!strcmp(file_name , snapshot_fn[i])){
			if(snapshot_count[i] == 0) {
				break; 
			}
			char *some_string = (char *)malloc(MAX_SIZE * sizeof(char) ) ;
			strcpy(some_string , "sss_") ;
			strcat(some_string , name_of_file(file_name) ) ;
			some_string[2] = 'A' + (snapshot_count[i]-1) ;
			some_string[3] = '_' ;
			printf("%s\n" , some_string) ;
			fp = fopen(some_string , "r") ;
			readrest(text , fp) ;
			fclose(fp) ;
			fob = fopen(file_name , "w") ;
			fprintf(fob , "%s" , text ) ;
			fclose(fob);
			snapshot_count[i]-- ;
			ok = 1 ;
			break ;
		}
	}
	if(!ok) printf("NO UNDO CAN BE DONE\n" , file_name) ;
	update_opened_text() ;
	calculate_text() ;
	saved_status = 0 ;
	oti = 0 ;
	otil = 0 ;
	cursor_x = 0 ;
	cursor_y = TEXT_START ;
	return 1 ;
}

int check_tree(char *command){
	if(strcmp(command,"tree")) return 0 ;
	int depth ;
	FILE *tmp_file = fopen("tmp_file.txt" , "w") ; 
	if(!scanf("%d" , &depth)) return 0 ;
	if(depth == -1) depth = 1000 ;
	if(depth < -1){
		printf("INVALID DEPTH\n") ;
		return 1 ;
	}
	if( opened_file != NULL ) save_opened_file() ;
	fclose(opened_file) ;
	fclose(fopen("untitled.txt" , "a")) ;
	opened_file = fopen("untitled.txt" , "r+") ;
	strcpy(opened_file_name , "untitled.txt") ;
	mustsave = 1 ;
	oti = 0 ;
	otil = 0 ;
	cursor_x = 0 ;
	cursor_y = TEXT_START ;
	dirtree_search("./root" , depth , depth+1 , tmp_file) ;
	dirtree_search("./root" , depth , depth+1 , opened_file) ;
	update_opened_text() ;
	fclose(tmp_file) ;
	if(check_arman()) arman_mode = 1 ;
	return 1 ;
}

int check_auto_indent(char *command){
	if(strcmp(command,"auto-indent")) return 0 ;
	char arg[MAX_SIZE] ;
	char file_name[MAX_SIZE] ;
	if( !auto_indent_opened_file ){
		scanf("%s" , arg) ;
		if(strcmp(arg , "-file")) return 0 ;
		get_address(file_name) ;
	}
	strcpy(file_name , opened_file_name) ;
	take_snapshot(opened_file_name , snapshot_count , snapshot_fn , &file_count) ;
	//Beep(500,100) ;
	auto_indent_opened_file = 0 ;
	char *text = (char*) malloc(MAX_SIZE*sizeof(char)) ;
	strcpy(text , opened_text) ;
	//FILE *fob = fopen(file_name , "r") ;
	//readrest(text , fob) ;
	//fclose(fob) ;
	int opening[MAX_SIZE] ;
	int closing[MAX_SIZE] ;
	int tmp_op[MAX_SIZE] ;
	int tmp_op_size = 0 ;
	int closing_size = 0 ;
	int opening_size = 0 ;
	
	stroneline(text) ;
	for(int i = 0 ; i < strlen(text) ; i++){
		if(text[i] == '{'){
			tmp_op[tmp_op_size] = i ;
			tmp_op_size++ ;
		}
		if(text[i] == '}' && tmp_op_size > 0){
			tmp_op_size-- ;
			opening[opening_size] = tmp_op[tmp_op_size] ;
			opening_size++ ;
			closing[closing_size] = i ;
			closing_size++ ;
		}
	}
	qsort(opening , opening_size , sizeof(int) , cmp) ;
	qsort(closing , closing_size , sizeof(int) , cmp) ;
	fclose(opened_file) ;
	FILE *fob = fopen(file_name , "w") ;
	int u = 0 ;
	while(text[u]==' ') u++ ;
	for(int op_index = 0 , cl_index = 0 , tab = 0  , start_of_line = 1 , i = u , space = 0 , ignore_space = 1 ; i < strlen(text) ; i++){
		
		if( cl_index < closing_size && i == closing[cl_index] ){
			if(start_of_line) print_tab(tab-1 , fob) ;
			if(!ignore_space){
				fprintf(fob , "\n") ;
				print_tab(tab-1 ,fob ) ;
			}
			fprintf(fob , "}\n") ;
			start_of_line = 1 ;
			ignore_space = 1 ;
			space = 0 ;
			tab-- ;
			cl_index++ ;
		}
		else if( op_index < opening_size && i == opening[op_index] ){
			if(start_of_line) print_tab(tab , fob) ;
			if(!ignore_space) fprintf(fob , " ") ;
			fprintf(fob , "{\n") ;
			start_of_line = 1;
			ignore_space = 1 ;
			space = 0 ;
			tab++ ;
			op_index++ ;
		}
		else if( text[i] == '\n' ){
			fprintf(fob , "\n") ;
			ignore_space = 1 ;
			space = 0 ;
			start_of_line = 1 ;
		}
		else{
			if(text[i] != ' '){
				if(start_of_line) print_tab(tab , fob) ;
				start_of_line = 0 ;
				for(int j = 0 ; j < space ; j++) fprintf(fob , " " ) ;
				space = 0 ;
				fprintf(fob , "%c" , text[i]) ;
				ignore_space = 0 ;
			}
			else{
				if(!ignore_space) space++ ;
				//if(!ignore_space) fprintf(fob , " " ) ;
			}
		}
	}
	fclose(fob) ;
	oti = 0 ;
	otil = 0 ;
	cursor_x = 0 ;
	cursor_y = TEXT_START ;
	saved_status = 0 ;
	opened_file = fopen(opened_file_name , "r+") ;
	update_opened_text() ;
	calculate_text() ;
	return 1 ;
}

int check_compare(char *command){
	if(strcmp(command , "compare")) return 0 ;
	char file_name1[MAX_SIZE] ;
	char file_name2[MAX_SIZE] ;
	get_address(file_name1) ;
	get_address(file_name2) ;
	FILE *fob1 = fopen(file_name1 , "r") ;
	FILE *fob2 = fopen(file_name2 , "r") ;
	int line_num1 =0;
	int line_num2 =0;
	char *text1 = (char*)malloc(MAX_SIZE*sizeof(char)) ;
	char *text2 = (char*)malloc(MAX_SIZE*sizeof(char)) ;
	readrest(text1,fob1);
	readrest(text2,fob2);
	fseek(fob1,0,SEEK_SET);
	fseek(fob2,0,SEEK_SET);
	for(int i = 0 ; i < strlen(text1) ; i++) if(text1[i] == '\n') line_num1++ ;
	for(int i = 0 ; i < strlen(text2) ; i++) if(text2[i] == '\n') line_num2++ ;
	int line = 1 ;
	while(1){
		if(NULL == fgets(text2 , MAX_SIZE , fob2)){ break ;}
		if(NULL == fgets(text1 , MAX_SIZE , fob1)){ break ;}
		line++ ;
		text1[strlen(text1)-1] = (text1[strlen(text1)-1] == '\n' ? '\0' : text1[strlen(text1)-1]) ;
		text2[strlen(text2)-1] = (text2[strlen(text2)-1] == '\n' ? '\0' : text2[strlen(text2)-1]) ;
		if(!strcmp(text1,text2)) continue ;
		char words1[1000][50] ;
		char words2[1000][50] ;
		int wordcount1 = getwords(text1 , words1) ;
		int wordcount2 = getwords(text2 , words2) ;
		int diff_word_count = 0 ;
		int diff_word_index = 0 ;
		for(int i = 0 ; i < min(wordcount2 , wordcount1) ; i++){
			if(strcmp(words1[i] , words2[i])){diff_word_index = i ; diff_word_count++ ;}
		}
		if(wordcount1 != wordcount2 ) diff_word_count = 0 ;
		printf("============ #%d ============\n",line) ;
		if(text1[strlen(text1)-1] == '\n') text1[strlen(text1)-1] = '\0';
		if(text2[strlen(text2)-1] == '\n') text2[strlen(text2)-1] = '\0';
	}

	if(line_num1 > line_num2){
		printf(">>>>>>>>>>>> #%d - #%d >>>>>>>>>>>>\n",line,line_num1) ;
		while(1){
			if(NULL == fgets(text1 , MAX_SIZE , fob1)){break ;}
			if(text1[strlen(text1)-1] == '\n') text1[strlen(text1)-1] = '\0';
			printf("%s\n" , text1) ;
			line++ ;
		}
	}
	if(line_num1 < line_num2){
		printf(">>>>>>>>>>>> #%d - #%d >>>>>>>>>>>>\n",line,line_num2) ;
		while(1){
			if(NULL == fgets(text2 , MAX_SIZE , fob2)){break ;}
			if(text2[strlen(text2)-1] == '\n') text2[strlen(text2)-1] = '\0';
			printf("%s\n" , text2) ;
			line++ ;
		}
	}

	return 1 ;

}

int check_open(char *command){
	if(strcmp(command ,"open")) return 0 ;
	char add[MAX_SIZE] ;
	get_address( add ) ;
	if( opened_file != NULL) save_opened_file() ;
	opened_file = fopen(add , "r") ;
	if( opened_file == NULL ){
		printf("WRONG ADDRESS\n") ;
		mustsave = 1 ;
		oti = 0 ;
		otil = 0 ;
		cursor_x = 0 ;
		cursor_y = TEXT_START ;
		return 0 ;
	}
	fclose(opened_file) ;
	opened_file = fopen(add , "r+") ;
	strcpy( opened_file_name , add ) ;
	readrest(opened_text , opened_file) ;
	calculate_text() ;
	oti = 0 ;
	mustsave = 0 ;
	otil = 0 ;
	cursor_x = 0 ;
	cursor_y = TEXT_START  ;
	return 1 ;
}

void copy_visual(){
	int string_size ;
	char *string = (char*) malloc(MAX_SIZE * sizeof(char)) ;
	string_size = 0 ;
	for(int i = min(selected_start , selected_end) ; i <= max(selected_start , selected_end) ; i++ ){
		string[string_size] = opened_text[i] ;
		string_size++ ;
	}
	string[string_size] = '\0' ;
	copytoclipboard(string) ;
	calculate_text() ;
	selected = 0 ;
}

void cut_visual(){
	saved_status = 0 ;
	int n ;
	int string_size ;
	char *string = (char*) malloc(MAX_SIZE * sizeof(char)) ;
	string_size = 0 ;
	for(int i = min(selected_start , selected_end) ; i <= max(selected_start , selected_end) ; i++ ){
		string[string_size] = opened_text[i] ;
		string_size++ ;
	}
	string[string_size] = '\0' ;
	copytoclipboard(string) ;
	n = abs( selected_end - selected_start + 1 ) ;
	for( int i = min(selected_end , selected_start) ; i+n < strlen(opened_text) ;i++ ){
		opened_text[i] = opened_text[ i + n ] ;
	}
	calculate_text() ;
	take_snapshot(opened_file_name , snapshot_count , snapshot_fn , &file_count) ;
	oti = 0 ;
	otil = 0 ;
	cursor_x = 0 ;
	cursor_y = TEXT_START ; 
	selected = 0 ;
}

void paste_visual(){
	saved_status = 0 ;
	char *string = (char*) malloc( MAX_SIZE * sizeof(char) ) ;
	OpenClipboard(0) ;
	HANDLE in = GetClipboardData(CF_TEXT) ;
	strcpy(string , (char*)in) ;
	CloseClipboard() ;
	insrtstrtostr( string , opened_text , oti ) ;
	calculate_text() ;
	take_snapshot(opened_file_name , snapshot_count , snapshot_fn , &file_count) ;
}

void jump_to(int x , int y){
	gotoxy(y+1,x+1) ;
}

int where_x(){
	return wherey() - 1 ;
}

int where_y(){
	return wherex() - 1 ;
}

int get_command(){
	
		int command_run = 0 ;
		char command[MAX_SIZE] ;

		scanf("%s" , command) ;
		//printf("give command : %s" , command) ;
		int (*check_function[])(char*) = { check_open , check_createfile , check_insertstr , check_cat , check_removestr , check_copystr , check_cutstr , check_pastestr , check_find , check_replace , check_grep , check_undo , check_auto_indent , check_compare ,check_tree } ;
		
		for(int i = 0 ; i < 15 ; i++){
			if( (*check_function[i])(command) ){
				command_run = 1 ;
			}
		}

		if(!strcmp("exit",command) || !strcmp( "q" , command ) ){

			if( mustsave ){
				jump_to(SCR_HEIGHT-1 , 0) ;
				textcolor(BLACK) ;
				textbackground(RED) ;
				printf( "IF YOU EXIT YOU WILL LOSE THIS FILE PERMANENTLY . ARE YOU SURE ? (Y/N)" ) ;
				char response = getch() ;
				while( response != 'y' && response != 'N' && response != 'n' && response != 'Y'  ) response = getch() ;
				textcolor(WHITE) ;
				textbackground(BLACK) ;
				if( response == 'n' || response == 'N' ) return 0 ;
				fclose(opened_file) ;
				remove("untitled.txt") ;
			}

			for(int i = 0 ; i < file_count  ; i++){
				for(int j = 0 ; j < snapshot_count[i] ; j++){
					char *name = (char*) malloc(MAX_SIZE * sizeof(char)) ;
					strcpy(name , "sss_") ;
					strcat(name , name_of_file(snapshot_fn[i])) ;
					name[2] = 'A' + j ;
					remove(name) ;
				}
			}
			remove("tmp_file.txt");
			return 1 ;
		}
		// save
		if( !strcmp(command , "save") ){
			if(mustsave){
				int x_tmp = where_x() ;
				int y_tmp = where_y() ;
				jump_to( SCR_HEIGHT-2 , 0 ) ;
				textbackground(RED) ;
				printf("YOU MUST USE saveas AND CHOOSE A NAME FIRST!") ;
				getch() ;
				textbackground(BLACK) ;
				jump_to(x_tmp , y_tmp) ;
			}
			save_opened_file() ;
			return 0 ;
		}
		// saveas
		if( !strcmp(command , "saveas") ){
			char name[MAX_SIZE] ;
			get_address(name) ;
			fclose(fopen(name , "a")) ;
			FILE *fp = fopen( name , "w") ;
			fprintf(fp , "%s" , opened_text) ;
			fclose(fp) ;
			mustsave = 0 ;
			
			fclose(opened_file) ;
			if( !strcmp(opened_file_name , "untitled.txt") ) remove("untitled.txt") ;

			opened_file = fopen(name , "r+") ;
			strcpy(opened_file_name , name) ;

			remove("untitled.txt") ;
			return 0 ;
		}
		// copy 
		if( MODE == VISUAL && !strcmp(command , "y") ){
			copy_visual() ;
			MODE = NORMAL ;
			return 0 ;
		}
		// paste
		if( MODE == NORMAL && !strcmp(command , "paste") ){
			paste_visual() ;
			return 0 ;
		}
		// cut 
		if( MODE == VISUAL && !strcmp(command , "d") ){
			cut_visual() ;
			MODE = NORMAL ;
			return 0 ;
		}
		if( command[0] == '/' ){
			char *string = (char*) malloc( MAX_SIZE * sizeof(char) ); 
			strcpy(string , command) ;
			find_in_of = 1 ;
			check_find(string+1) ;
		}

		return 0 ;
}

void printscr( char *text , int x , int y , int color_t , int color_b ){
	int x_tmp = where_x() ;
	int y_tmp = where_y() ;
	jump_to(x , y) ;
	textcolor(color_t) ;
	textbackground(color_b) ;
	printf("%s" , text) ;
	textcolor(WHITE) ;
	textbackground(BLACK) ;
	jump_to(x_tmp , y_tmp) ;
}

int jump_to_text(int x , int y){
	// we are able to point to any charachter even the NULL in the end.
	if( y < SCR_WIDTH && y >= TEXT_START && x < SCR_HEIGHT-2 && x >= 0 && x == SCR_HEIGHT - 6 && x + FIRST_LINE_NUM <= opened_text_line_count ){
		FIRST_LINE_NUM++;
		return 0 ;
	}

	if( y < SCR_WIDTH && y >= TEXT_START && x < SCR_HEIGHT-2 && x >= 0 && x == 3 && FIRST_LINE_NUM > 1 ){
		FIRST_LINE_NUM--;
		return 0 ;
	}

	int change = 0 ;
	
	if( y < SCR_WIDTH && y >= TEXT_START && x < SCR_HEIGHT-2 && x >= 0 ){
		if( x + FIRST_LINE_NUM > opened_text_line_count ){ return 0 ;}
		if( y > TEXT_START + opened_text_line_length[x + FIRST_LINE_NUM - 1] - 1 ){
			y = TEXT_START + opened_text_line_length[x + FIRST_LINE_NUM - 1] - 1 ;
		}
		if( cursor_x != x ){
			cursor_x = x ;
			change = 1 ;
		}
		if( cursor_y != y ){
			cursor_y = y ;
			change = 1 ;
		}
		return change ;
	}


	return 0 ;
}

void write_file_name(){
	int x_tmp = where_x() ;
	int y_tmp = where_y() ;
	jump_to(SCR_HEIGHT-2 , 0) ;
	
	switch(MODE){
		case NORMAL :
			textbackground(MAGENTA) ;
			printf("NORMAL") ;
			textbackground(BLACK) ;
			printf(" : %s", opened_file_name) ;
			break ;
		case VISUAL :
			textbackground(MAGENTA) ;
			printf("VISUAL") ;
			textbackground(BLACK) ;
			printf(" : %s", opened_file_name) ;
			break ;
		case INSERT :
			textbackground(MAGENTA) ;
			printf("INSERT") ;
			textbackground(BLACK) ;
			printf(" : %s", opened_file_name) ;
			break ;
	}
	
	if(!saved_status) printf(" + "); 

	jump_to(x_tmp , y_tmp) ;
}

void write_text(char *text){
	int x_tmp = where_x() ;
	int y_tmp = where_y() ;
	jump_to(0,TEXT_START) ;
	for(int selected_index = 0 , i = 0 , u = 0 ; i < strlen(text) ; i++){
		while( u < FIRST_LINE_NUM-1 && i < strlen(text) ){
			if(text[i] == '\n'){
				u++ ;
			}
			i++ ;
			continue ;
		}
		if(text[i] == '\n'){
			if(wherey()+1 > 28) break ;
			gotoxy( TEXT_START + 1 , wherey()+1 ) ;
			continue ;
		}
		if( min(selected_start , selected_end) == i && selected ) textbackground(BLUE) ;
		if( find_highlight_status && selected_index < find_count && i == find_highlight[selected_index][0] ){
			textbackground(BLUE) ;
		}
		printf("%c",text[i]) ;
		if( find_highlight_status && selected_index < find_count && i == find_highlight[selected_index][1] ){
			textbackground(BLACK) ;
			selected_index++ ;
		}
		if( max(selected_end , selected_start) == i && selected ) textbackground(BLACK) ;
	}
	jump_to(x_tmp , y_tmp) ;
}

void drawscr(){
	clrscr() ;

	// DEBUG ?
	printscr( itoa( opened_text_line_count , (char*)malloc(MAX_SIZE * sizeof(char)) , 10 ) , 1 , 105 , WHITE , BLACK) ;
	printscr( itoa( oti , (char*)malloc(MAX_SIZE * sizeof(char)) , 10 ) , 2 , 105 , WHITE , BLACK) ;
	printscr(  itoa( otil , (char*)malloc(MAX_SIZE * sizeof(char)) , 10 ) , 3 , 105 , WHITE , BLACK) ;
	printscr(  itoa( opened_text_line_length[opened_text_line_count-1] , (char*)malloc(MAX_SIZE * sizeof(char)) , 10 ) , 4 , 105 , WHITE , BLACK) ;
	// NO DEBUG ?
	
	for(int i = 0 ; i < SCR_HEIGHT-2 ; i++){
		jump_to(i , 0) ;
		textbackground(BLACK) ;
		textcolor(WHITE) ;
		printf("%5d" , i+FIRST_LINE_NUM ) ;
		printf("  ") ;
		textcolor(WHITE) ;
		textbackground(BLACK) ;
		for(int j = TEXT_START ; j < SCR_WIDTH ;j++){
			printf(" ") ;
		}
		textbackground(WHITE) ;
		printf(" ") ;
		textbackground(BLACK) ;
	}
	for(int i = SCR_HEIGHT-2 ; i < SCR_HEIGHT-1  ; i++){
		jump_to(i , 0) ;
		textbackground(BLUE) ;
		for(int j = 0 ; j < SCR_WIDTH ;j++){
			printf( " " )  ;
		}
		textbackground(WHITE) ;
		printf(" ") ;
		textbackground(BLACK) ;
	}
	
}

void select_update( int index ){
	if(!selected){
		selected_start = index ;
		selected_end = index ;
		selected = 1 ;
	}
	else{
		selected_end = index ;
	}
}

void normal_input_control(char input){

	switch(input){
		int x_tmp ;
		int y_tmp ;
		case 'k' :
			find_highlight_status = 0 ;
			if(jump_to_text(cursor_x-1 , cursor_y )){
				oti -= otil+opened_text_line_length[cursor_x] ;
				otil = min( opened_text_line_length[cursor_x]-1 , otil ) ;
				oti += otil ;
			}
			break ;
		case 'l' :
			find_highlight_status = 0 ;
			if(jump_to_text(cursor_x , cursor_y+1)) oti++,otil++ ;
			break ;
		case 'j' :
			find_highlight_status = 0 ;
			if(jump_to_text(cursor_x+1 , cursor_y)){
				oti -= otil-opened_text_line_length[cursor_x-1] ;
				otil = min( opened_text_line_length[cursor_x]-1 , otil ) ;
				oti += otil ;
			}
			break ;
		case 'h' :
			find_highlight_status = 0 ;
			if(jump_to_text(cursor_x , cursor_y-1)) oti--,otil-- ;
			break ;
		case 'i' :
			if( opened_file == NULL ){
				fopen("untitled.txt" , "a") ;
				strcpy(opened_file_name , "untitled.txt") ;
				fclose(opened_file) ;
				fclose(fopen("untitled.txt","w")) ;
				opened_text[0] = '\0' ;
				opened_file = fopen("untitled.txt" , "r+") ;
				mustsave = 1 ;
				calculate_text() ;
			}
			find_highlight_status = 0 ;
			MODE = INSERT ;
			break ;
		case 'v' :
			find_highlight_status = 0 ;
			MODE = VISUAL ;
			break ;
		case ':' :
			find_highlight_status = 0 ;
			jump_to(SCR_HEIGHT-1 , 0) ;
			delline() ;
			EXIT = get_command() ;
			jump_to(cursor_x , cursor_y) ;
			break ;
		case '/' :
			find_highlight_status = 0 ;
			jump_to(SCR_HEIGHT-1 , 0) ;
			delline() ;
			EXIT = get_command() ;
			jump_to(cursor_x , cursor_y) ;
			break ;
		case '=' :
			find_highlight_status = 0 ;
			auto_indent_opened_file = 1 ;
			check_auto_indent("auto-indent") ;
			break ;
		case 'u' :
			find_highlight_status = 0 ;
			undo_opened_file = 1 ;
			check_undo("undo") ;
			break ;
		case 'n' :
			if( !find_highlight_status ) break ;
			find_pos(find_highlight[cur_fh_i][0] , &cursor_x , &cursor_y) ;
			oti = find_highlight[cur_fh_i][0];
			otil = cursor_y - TEXT_START ;
			cur_fh_i++;
			cur_fh_i %= find_count; 
			break ;
		case 19 :
			find_highlight_status = 0 ;
			save_opened_file() ;
			break ;
		default :
			find_highlight_status = 0 ;
			break ;
		
	}
}

void insert_input_control(char input){
	find_highlight_status = 0 ;
	switch(input){
		int x_tmp ;
		int y_tmp ;
		case -32 :
			switch(getch()){
				case 72 :
					if(jump_to_text(cursor_x-1 , cursor_y )){
						oti -= otil+opened_text_line_length[ FIRST_LINE_NUM - 1 + cursor_x ] ;
						otil = min( opened_text_line_length[ FIRST_LINE_NUM - 1 + cursor_x ]-1 , otil ) ;
						oti += otil ;
					}
					return ;
				case 77 :
					if(jump_to_text(cursor_x , cursor_y+1)) oti++,otil++ ;
					return ;
				case 80 :
					if(jump_to_text(cursor_x+1 , cursor_y)){
						oti -= otil-opened_text_line_length[FIRST_LINE_NUM - 1 + cursor_x-1] ;
						otil = min( opened_text_line_length[FIRST_LINE_NUM - 1 + cursor_x]-1 , otil ) ;
						oti += otil ;
					}
					return ;
				case 75 :
					if(jump_to_text(cursor_x , cursor_y-1)) oti--,otil-- ;
					return ;
			}
		case 27 : // ESC
			MODE = NORMAL ;
			return ;
		case 19 :
			save_opened_file() ;
			return ;
	}
	if( input >= 32 ){
		saved_status = 0 ;
		insrtchtostr( opened_text , input , oti ) ;
		oti++ ;
		otil++ ;
		cursor_y++ ;
		calculate_text() ;
	}
	if( input == 8 ){
		if(oti <= 0) return ; 
		saved_status = 0 ;
		strdel( opened_text , oti-1 ) ;
		oti-- ;
		calculate_text() ;
		int x_tmp , y_tmp ;
		find_pos( oti , &x_tmp , &y_tmp ) ;
		jump_to_text( x_tmp-(FIRST_LINE_NUM-1) , y_tmp) ;
		otil = cursor_y - TEXT_START ;
	}
	if( input == 13 ){
		saved_status = 0 ;
		insrtchtostr( opened_text , '\n' , oti ) ;
		calculate_text() ;
		oti++ ;
		otil = 0 ;
		//cursor_x++ ;
		//cursor_y = TEXT_START ;
		jump_to_text( cursor_x + 1 , TEXT_START ) ;
	}
}

void visual_input_control(char input){
	find_highlight_status = 0 ;
	int x_tmp ;
	int y_tmp ;
	switch(input){
		case 'k' :
			if(jump_to_text(cursor_x-1 , cursor_y )){
				oti -= otil+opened_text_line_length[ FIRST_LINE_NUM - 1 + cursor_x] ;
				otil = min( opened_text_line_length[ FIRST_LINE_NUM - 1 + cursor_x]-1 , otil ) ;
				oti += otil ;
			}
			selected = 0 ;
			return ;
		case 'l' :
			if(jump_to_text(cursor_x , cursor_y+1)) oti++,otil++ ;
			selected = 0 ;
			return ;
		case 'j' :
			if(jump_to_text(cursor_x+1 , cursor_y)){
				oti -= otil-opened_text_line_length[FIRST_LINE_NUM - 1 + cursor_x-1] ;
				otil = min( opened_text_line_length[FIRST_LINE_NUM - 1 + cursor_x]-1 , otil ) ;
				oti += otil ;
			}
			selected = 0 ;
			return ;
		case 'h' :
			if(jump_to_text(cursor_x , cursor_y-1)) oti--,otil-- ;
			selected = 0 ;
			return ;
		case 27 : 
			selected = 0 ;
			MODE = NORMAL ;
			return ;
		case 'K' : // UP
			if(jump_to_text(cursor_x-1 , cursor_y )){
				if(!selected){
					select_update(oti-1) ;
				}
				oti -= otil+opened_text_line_length[ FIRST_LINE_NUM - 1 + cursor_x ] ;
				otil = min( opened_text_line_length[ FIRST_LINE_NUM - 1 + cursor_x ] - 1 , otil ) ;
				oti += otil ;
				select_update( oti - ( selected_start < oti ? 1 : 0) ) ;
			}
			return ;
		case 'L' : // RIGHT
			if(jump_to_text(cursor_x , cursor_y+1)){
				if(!selected) select_update(oti) ;
				oti++,otil++ ;
				select_update( oti - ( oti > selected_start ? 1 : 0) ) ;
			}
			return ;
		case 'J' : // DOWN
			if(jump_to_text(cursor_x+1 , cursor_y)){
				if(!selected) select_update(oti) ;
				oti -= otil-opened_text_line_length[FIRST_LINE_NUM - 1 + cursor_x-1] ;
				otil = min( opened_text_line_length[FIRST_LINE_NUM - 1 + cursor_x]-1 , otil ) ;
				oti += otil ;
				select_update(oti - ( oti > selected_start ? 1 : 0 ) ) ;
			}
			return ;
		case 'H' : // LEFT
			if(jump_to_text(cursor_x , cursor_y-1)){
				if(!selected) select_update(oti) ; 
				oti--,otil-- ;
				select_update(oti - ( selected_start < oti ? 1 : 0 ) ) ;
			}
			return ;
		case '/' :
			jump_to(SCR_HEIGHT-1 , 0) ;
			delline() ;
			EXIT = get_command() ;
			jump_to(cursor_x , cursor_y) ;
			break ;
		case ':' :
			jump_to(SCR_HEIGHT-1 , 0) ;
			delline() ;
			EXIT = get_command() ;
			jump_to( cursor_x , cursor_y ) ;
			break ;
		case 3 :
			copy_visual() ;
			MODE = NORMAL ;
			break ;
		case 24 :
			cut_visual() ;
			MODE = NORMAL ;
			break ;
		case 22 :
			paste_visual() ;
			MODE = NORMAL ;
			break ;
		case 19 :
			save_opened_file() ;
			break ;
		case -32 :
			if( getch() == 83 ){
				selected = 0 ;
				take_snapshot(opened_file_name , snapshot_count , snapshot_fn , &file_count) ;
				saved_status = 0 ;
				int n = abs( selected_end - selected_start ) ;
				for(int i = min(selected_end , selected_start) ; i + n <= strlen(opened_text) ;i++ ){
					opened_text[i] = opened_text[ i + n ];
				}
				otil = 0 ;
				oti = 0 ;
				cursor_x = 0 ;
				cursor_y = TEXT_START ;
				calculate_text() ;
			}
			break ;
		
	}
}

int main(){
	opened_file_name = (char*) malloc(MAX_SIZE * sizeof(char)) ;
	opened_text = (char*) malloc(MAX_SIZE * sizeof(char)) ;
	// RESET SNAPSHOT-RELATED VARIABLES
	RESET_SS_VARS();
	// CREATING TMP_FILE
	fclose(fopen("tmp_file.txt" , "w+")) ;
	char input = '0' ;
	while(1){
		switch(MODE){
			case NORMAL :
				normal_input_control(input) ;
				break ;
			case VISUAL :
				visual_input_control(input) ;
				break ;
			case INSERT :
				insert_input_control(input) ;
				break ;
		}
		if(EXIT){ break ;}
		drawscr() ;
		if(opened_file != NULL){
			textcolor(WHITE) ;
			write_text(opened_text) ;
			textcolor(WHITE) ;
			write_file_name() ;
		}
		jump_to(cursor_x , cursor_y) ;
		input  = getch() ;
	}
	fclose(opened_file) ;
}