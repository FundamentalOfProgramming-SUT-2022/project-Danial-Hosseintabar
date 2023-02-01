#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <conio.h>

#include "functions.c"

#define SS_MAX 100
#define FN_MAX 50

// GLOBAL ARRAYS FOR UNDO FUNCTIONING
int snapshot_count[SS_MAX] ;
char *snapshot_fn[SS_MAX] ;
int file_count = 0 ;

// GLOBAL VARIABLES FOR ARMAN COMMAND
int arman_mode = 0 ;
char arman_str[MAX_SIZE] ;

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
	get_address(file_name) ;
	
	if(!arman_mode){get_str_arg(string) ;}
	else{
		FILE *tmp_file = fopen("tmp_file.vim" , "r") ;
		fseek(tmp_file , 0 , SEEK_SET) ;
		readrest(string , tmp_file) ;
		fclose(tmp_file) ;
		arman_mode = 0 ;
	}
	scanf("%s" , opt ) ;
	if(strcmp(opt , "-pos")) return 0 ;
	if(2!=scanf("%d:%d" , &line_pos , &char_pos)) return 0 ;
	
	// debug : printf("file : %s , str : %s , line_pos : %d , char_pos : %d \n" , file_name , string , line_pos , char_pos ) ;

	// TAKING SNAPSHOT

	take_snapshot(file_name , snapshot_count , snapshot_fn , &file_count) ;
	
	// INSERTING

	FILE *fob = fopen(file_name , "r+" ) ;
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

// \n \* not supported
// byword not supported
int check_find(char *command){
	if(strcmp(command , "find")) return 0 ;
	char string[MAX_SIZE] ;
	char arg[MAX_SIZE] ;
	char file_name[MAX_SIZE] ;
	
	if(!arman_mode) get_str_arg(string) ;
	else{
		FILE *tmp_file = fopen("tmp_file.vim","r") ;
		fseek(tmp_file , 0 , SEEK_SET) ;
		readrest(string,tmp_file) ;
		arman_mode = 0 ;
		fclose(tmp_file) ;
	}

	scanf("%s" , arg) ;
	if(strcmp(arg , "-file")) return 0 ;
	scanf("%s" , file_name) ;
	int at = -1 ;
	int mask = 0 ; // mask = count , at , byword , all
	char c = getchar() ;
	
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

	char *text = (char*) malloc(MAX_SIZE * sizeof(char) ) ;
	FILE *fob = fopen(file_name , "r") ;
	readrest(text , fob) ;
	fclose(fob) ;
	
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

	
	FILE *tmp_file = fopen("tmp_file.vim" , "w") ;

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
	fclose(tmp_file) ;
	return 1 ;

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
		printf(" string : %s",string1) ;
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

	fob = fopen(file_name , "w") ;
	for(int u = 0 , i = 0 ; i < strlen(text) ; i++){
		if(ans[u] == i){
			u++ ;
			fprintf(fob , "%s" , string2) ;
			i+= strlen(string1) - 1 ;
			continue ;
		}
		fprintf(fob , "%c" , text[i] ) ;
	}
	fclose(fob) ;

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
		FILE *tmp_file = fopen("tmp_file.vim" , "r") ;
		fseek(tmp_file , 0 , SEEK_SET) ;
		readrest(string , tmp_file) ;
		fclose(tmp_file) ;
	}
	if(!arman_mode){
		scanf("%s" , arg) ;
		if(strcmp("-files" , arg)) return 0 ;
	}
	if(arman_mode) arman_mode = 0 ;
	FILE *tmp_file = fopen("tmp_file.vim" , "w") ;
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
	char file_name[MAX_SIZE] ;
	scanf("%s" , arg) ;
	if(strcmp(arg , "-file")) return 0 ;
	get_address(file_name) ;
	FILE *fob ;
	FILE *fp ;
	for(int i = 0 ; i < file_count ; i++){
		if(!strcmp(file_name , snapshot_fn[i])){
			if(snapshot_count[i] == 0) {
				break; 
			}
			char *some_string = (char *)malloc(MAX_SIZE * sizeof(char) ) ;
			strcpy(some_string , "sss") ;
			strcat(some_string , file_name) ;
			some_string[2] = 'A' + (snapshot_count[i]-1) ;
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
	return 1 ;
}

int check_tree(char *command){
	if(strcmp(command,"tree")) return 0 ;
	int depth ;
	FILE *tmp_file = fopen("tmp_file.vim" , "w") ; 
	if(!scanf("%d" , &depth)) return 0 ;
	if(depth == -1) depth = 1000 ;
	if(depth < -1){
		printf("INVALID DEPTH\n") ;
		return 1 ;
	}
	dirtree_search("./root" , depth , depth+1 , tmp_file) ;
	fclose(tmp_file) ;
	if(check_arman()) arman_mode = 1 ;
	return 1 ;
}

int check_auto_indent(char *command){
	if(strcmp(command,"auto-indent")) return 0 ;
	char arg[MAX_SIZE] ;
	char file_name[MAX_SIZE] ;
	scanf("%s" , arg) ;
	if(strcmp(arg , "-file")) return 0 ;
	get_address(file_name) ;
	
	take_snapshot(file_name , snapshot_count , snapshot_fn , &file_count) ;

	char *text = (char*) malloc(MAX_SIZE*sizeof(char)) ;
	FILE *fob = fopen(file_name , "r") ;
	readrest(text , fob) ;
	fclose(fob) ;
	int opening[MAX_SIZE] ;
	int closing[MAX_SIZE] ;
	int tmp_op[MAX_SIZE] ;
	int tmp_op_size = 0 ;
	int closing_size = 0 ;
	int opening_size = 0 ;
	
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
	
	fob = fopen(file_name , "w") ;
	int u = 0 ;
	while(text[u]==' ') u++ ;
	for(int op_index = 0 , cl_index = 0 , tab = 0  , i = u , space = 0 , a = 0 , ignore_space = 1 ; i < strlen(text) ; i++){
		
		if( cl_index < closing_size && i == closing[cl_index] ){
			if( a ){
				fprintf(fob , "\n") ;
				print_tab(tab-1 , fob) ;
			}
			a = 0 ;
			fprintf(fob , "}\n") ;
			ignore_space = 1 ;
			space = 0 ;
			tab-- ;
			cl_index++ ;
			print_tab(tab , fob) ;
		}
		else if( op_index < opening_size && i == opening[op_index] ){
			fprintf(fob , " ") ;
			fprintf(fob , "{\n") ;
			ignore_space = 1 ;
			space = 0 ;
			tab++ ;
			op_index++ ;
			a = 1 ;
			print_tab(tab , fob) ;
		}
		else if( text[i] == '\n' ){
			fprintf(fob , "\n") ;
			ignore_space = 1 ;
			space = 0 ;
			print_tab(tab,fob) ;
			a = 1 ;
		}
		else{
			if(text[i] != ' '){
				for(int j = 0 ; j < space ; j++) fprintf(fob , " " ) ;
				space = 0 ;
				fprintf(fob , "%c" , text[i]) ;
				ignore_space = 0 ;
			}
			else{
				if(!ignore_space) space++ ;
				//if(!ignore_space) fprintf(fob , " " ) ;
			}
			a = 1 ;
		}
	}
	fclose(fob) ;
	
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

int main(){
	printf("FOP 2022 PROJECT : Danial Hosseintabar\n") ;
	// RESET SNAPSHOT-RELATED VARIABLES
	RESET_SS_VARS();
	// OPENING TMP FILE
	fopen("tmp_file.vim" , "w+") ;
	// PROGRAM LOOP
	while(1){
		int command_run = 0 ;
		char command[MAX_SIZE] ;

		scanf("%s" , command) ;
		
		if(!strcmp("exit",command)) break ;
		
		int (*check_function[])(char*) = { check_createfile , check_insertstr , check_cat , check_removestr , check_copystr , check_cutstr , check_pastestr , check_find , check_replace , check_grep , check_undo , check_auto_indent , check_compare ,check_tree } ;
		
		for(int i = 0 ; i < 14 ; i++){
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
	for(int i = 0 ; i < file_count  ; i++){
		for(int j = 0 ; j < snapshot_count[i] ; j++){
			char *name = (char*) malloc(MAX_SIZE * sizeof(char)) ;
			strcpy(name , "sss") ;
			strcat(name , snapshot_fn[i]) ;
			name[2] = 'A' + j ;
			remove(name) ;
		}
	}
	remove("tmp_file.vim");
}
