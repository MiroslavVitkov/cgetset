/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <sir.vorac@gmail.com> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. 
 * Miroslav Vitkov 2013
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_TO_FILE_LENGTH_MAX 500
#define LINE_LENGTH_MAX 120
#define IDENTIFIER_LENGTH_MAX 40
#define FOREVER 1
#define NEVER 0
#define LINE_DELIMIT '\n'

//Extracts a line, delimited by LINE_DELIMIT, advances file pointer to after the delimiter.
//Returns characters count, excluding trailing zero. Negative numbers returned on error.
//Throwes error on non-printable characters.
int getline(FILE *const fileIn, char *const line);

//Extracts a word, delimited by any character in "delimit". advances line pointer to after the delimiter
//Return value: word length, not counting terminating \0
int getword(char **line, char *const word, const char* const delimit); 

//Attempts to open $(dirTemplates)/$(type). On failure, try to open $(dirTemplates)/default.
//On failure, returns a negative value (error).
//On successs, creates/overwrites $(fnameOut).h and $(fnameOut).c for appending. 
//Only variable declarations are written to (fnameOut).h, one variable per line;
//For $(fnameOut).c, characters are read from $(dirTemplates)/$(type) (or $(dirTemplates)/default) one by one.
//If "@" is encountered, it is exchanged for $(var). Characters are appended to $(fnameOut).c. 
//Return value is 0.
int generate(const char *const type, const char *const var, const char *const fnameOut, const char *const dirTemplates);

//As writing to output files is in append mode, this function deletes them, if existing, at program startup.
int clearOutputFiles(const char *const fnameOut);

int main(int argc, char **argv){
	if(argc != 3){
		printf("usage: cgetset <input file> <templates folder>\n");
		exit(-1);
	}

	//Configuration
	char fnameIn[PATH_TO_FILE_LENGTH_MAX];
	memcpy(fnameIn, argv[1], strlen(argv[1]));
	char dirTemplates[PATH_TO_FILE_LENGTH_MAX];
	memcpy(dirTemplates, argv[2], strlen(argv[2]));
	char fnameOut[PATH_TO_FILE_LENGTH_MAX] = "out";
	char delimit[] = ", };\t";				//important config constant

	//Other declarations
	typedef struct{
		int num;
		int len;
		char buff[LINE_LENGTH_MAX];
	}Line_s;
	Line_s line_o;
	line_o.num = 1;
		
	typedef struct{
		int len;
		char buff[IDENTIFIER_LENGTH_MAX];
	}word_s;
	word_s word, type;
		
	//delete any existing output files
	if(clearOutputFiles(fnameOut)){
		printf("Creating some output files failed. Check permissions, target directory existence, etc\n" );
		exit(-1);
	}
	
	//Open the file with the variables declared
	FILE *fileIn = fopen(fnameIn, "r");
	if(fileIn == NULL){
		fprintf(stderr, "Error opening input file at: %s\n", fnameIn);
		exit(-1);
	}

	//Main program loop
	//read file line by line, generating a new setter/getter on the fly for every variable
	do{

		line_o.len = getline(fileIn, line_o.buff);
		if(line_o.len == EOF){
			exit(0);	//end of input file
		} else if(line_o.len == 0){
			continue;	//empty line, skip to the next
		} else if(line_o.len < 0){
			fprintf(stderr, "Error reading line. Errno: %d Lineno: %i\n", line_o.len, line_o.num);
			exit(-1);
		}
		line_o.num++;

		//Type
		//As getword() advances the line pointer, it is essential that the line iterator is reinitialized on every iteration.
		char *itLine = line_o.buff;
		word.len = getword(&itLine, word.buff, delimit);
		if(word.len == 0){
			continue;			//empty line, skip to the next
		} else if(word.len < 0){
			fprintf(stderr, "Extracting a word from line failed with errno %i\n", word.len);
			fprintf(stderr, "The line buffer contained:\n%s", line_o.buff);
			fprintf(stderr, "\nThe line pointer contained:\n%s", itLine);
			exit(-1);
		}
		memcpy((void*)type.buff, (void*)&word.buff, IDENTIFIER_LENGTH_MAX);	
		
		//Variables
		while((word.len = getword(&itLine, word.buff, delimit)) != 0){
			if(word.len < 0){
					fprintf(stderr, "Syntactic error suspected on line on line ", word.len);
					fprintf(stderr, "The line buffer contained:\n%s", line_o.buff);
					fprintf(stderr, "\nThe line pointer contained:\n%s", itLine);
					exit(-1);
			}
			generate(type.buff, word.buff, fnameOut, dirTemplates);
		};	//variables
	}while(line_o.len > 0);
	exit(0);
}	//main()


//extracts a line, delimited by LINE_DELIMIT, advances file pointer to after the delimiter
//return value: 0 - success, != 0 - error
int getline(FILE *const fileIn, char *const line){
	int count;
	char c;
	for(count = 0, c = fgetc(fileIn);
	count < LINE_LENGTH_MAX - 1 && c != EOF && c != LINE_DELIMIT;
	count++, c = fgetc(fileIn)){
		if(!isprint(c) && c != '\t'){
				fprintf(stderr, "Encountered not printable character: %i\n", (int)c);
				exit(-1);
		}
		line[count] = c;
	}
	line[count] = 0;
	return count;
}	

//extracts a word, delimited by WORD_DELIMIT, advances line pointer to after the delimiter
//delimiters is a string of word delimitting cgaracters, not counting the trailing \0
//return value: word len, not counting terminating \0
int getword(char **line, char *const word, const char *const delimit){
	//words begin with a letter or an _
	while(FOREVER){
		char d = *line[0];
		if(d == LINE_DELIMIT || d == '\0'){
			return 0;		//empty line
		} else if(!isalpha(d) && d !='_'){
			(*line)++;		//ignore character
		} else {
			break;			//OK
		}
	};

	//extract the word
	char c;
	char *itWord = word;
	const char *const itWordEnd = word + IDENTIFIER_LENGTH_MAX - 1;	//last printable character position
	do{
		c = *line[0];
		if(strchr(delimit, c) != NULL ||	//c is a delimiter
		itWord >= itWordEnd){				//no more space in word buffer
			break;
		} else if(!isprint(c)){
			//error
		}
		*itWord = c;
		itWord++; (*line)++;
	}while(FOREVER);
	*itWord = 0;
	
	return itWord - word;
}

//Attempts to open $(dirTemplates)/$(type).
//On failure, returns a negative value (error).
//On successs, opens $(fnameOut) for appending. Reads characters from the infile one by one. If "@" is encountered, it is
//exchanged for $output file.
int generate(const char *const type, const char *const var, const char *const fnameOut, const char *const dirTemplates){
	//open outfiles
	char fnameH[PATH_TO_FILE_LENGTH_MAX + 2];
	memcpy(fnameH, fnameOut, PATH_TO_FILE_LENGTH_MAX);
	strcat(fnameH, ".h");
	FILE *fileH = fopen(fnameH, "a");
	if(fileH == NULL){
		fprintf(stderr, "Error appending/creating output file at: %s\n", fnameH);
		return(-1);
	}
	
	char fnameC[PATH_TO_FILE_LENGTH_MAX + 2];
	memcpy(fnameC, fnameOut, PATH_TO_FILE_LENGTH_MAX);
	strcat(fnameC, ".c");
	FILE *fileC = fopen(fnameC, "a");
	if(fileC == NULL){
		fprintf(stderr, "Error appending/creating output file at: %s\n", fnameC);
		return(-1);
	}
	
	//search for a template file
	char fnameTarget[PATH_TO_FILE_LENGTH_MAX];
	memcpy(fnameTarget, dirTemplates, PATH_TO_FILE_LENGTH_MAX);
	if(strlen(fnameTarget) + strlen(var) + 1 > PATH_TO_FILE_LENGTH_MAX){
		return -1;	//buffer overflow  would result
	}
	strcat(fnameTarget, "\\");
	strcat(fnameTarget, type);
	
	char fnameTargetDefault[PATH_TO_FILE_LENGTH_MAX];
	memcpy(fnameTargetDefault, dirTemplates, PATH_TO_FILE_LENGTH_MAX);
	if(strlen(fnameTargetDefault) + strlen("default") + 1 > PATH_TO_FILE_LENGTH_MAX){
		return -1;	//buffer overflow  would result
	}
	strcat(fnameTargetDefault, "\\");
	strcat(fnameTargetDefault, "default");
	
	FILE *fileTarg = fopen(fnameTarget, "r");
	if(fileTarg == NULL){
		fileTarg = fopen(fnameTargetDefault, "r");
		if(fileTarg == NULL){
			return -1;
		}
	}
	
	//pipe content to outfiles
	fprintf(fileH, "%s %s;\n", type, var);
	//fprintf(fileC, "%s %s = 0;\n", type, var);
	do{
		char c;
		c = fgetc(fileTarg);
		if(c == EOF){
			break;
		} else if(c == '@'){
			for(const char *it = var; *it != '\0'; it++){
				fputc((int)*it, fileC);
			}
		} else{
			fputc(c, fileC);
		}
	}while(FOREVER);
	fputc((int)LINE_DELIMIT, fileC);
	
	//clean up
	fclose(fileC); fclose(fileH);
}

//As writing to output files is in append mode, this function deletes them, if existing, at program startup.
int clearOutputFiles(const char *const fnameOut){
	char fnameH[PATH_TO_FILE_LENGTH_MAX + 2];
	memcpy(fnameH, fnameOut, PATH_TO_FILE_LENGTH_MAX);
	strcat(fnameH, ".h");
	FILE *fileH = fopen(fnameH, "w");
	if(fileH == NULL){
		return(-1);
	}
	fclose(fileH);
	
	char fnameC[PATH_TO_FILE_LENGTH_MAX + 2];
	memcpy(fnameC, fnameOut, PATH_TO_FILE_LENGTH_MAX);
	strcat(fnameC, ".c");
	FILE *fileC = fopen(fnameC, "w");
	if(fileC == NULL){
		return(-2);
	}
	fclose(fileC);
	
	return 0;
}