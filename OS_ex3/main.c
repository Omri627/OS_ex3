// Omri Sak 205892615
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFF_SIZE 1
#define SYS_ERROR "Error in system call\n"
#define SYS_ERROR_CHARS 21
#define ADDITION 100

/**
 * struct for keeping the chars
 */
typedef struct Text {
    char* charsArray;
} Text;
/**
 * check if there is enough arguments
 * @param args
 * @return -1 if not 0 if ok
 */
int checkParams(int args) {
    if (args != 3) {
        write(STDERR_FILENO,"Error: program needs two path files\n", 36);
        return -1;
    }
    return 0;
}

/**
 * open two files
 * @param argv the paths
 * @param fdin1 FD for file one
 * @param fdin2 FD for file two
 * @return 1 if open successfully, else 0
 */
int openFiles(char **argv, int* fdin1, int* fdin2) {

    if ((*fdin1 = open(argv[1],O_RDONLY)) < 0) {
        write(STDERR_FILENO,SYS_ERROR,SYS_ERROR_CHARS);
        return 0;
    }
    if ((*fdin2 = open(argv[2],O_RDONLY)) < 0) {
        write(STDERR_FILENO,SYS_ERROR,SYS_ERROR_CHARS);
        return 0;
    }
    return 1;
}
/**
 * check if the files are the same
 * @param fd1 file1
 * @param fd2 file2
 * @return return 1 if same, else 0
 */
int isFileSame(int fd1, int fd2) {
    int ans1, ans2;
    char c1, c2;
    while (1) {
        ans1 = read(fd1, &c1, BUFF_SIZE);
        ans2 = read(fd2, &c2, BUFF_SIZE);

        if (c1 != c2) //not equals chars
            return 0;
        if ((ans1 == 0 && ans2 != 0) || (ans1 != 0 && ans2 == 0)) //one file is shorter then other
            return 0;
        if ((ans1 == 0) && (ans2 == 0)) //reach end of files
            break;
    }
    return 1;
}
/**
 * get text from file
 * @param text text contain pointer to array of char
 * @param fd
 */
void getText(Text* text,int fd) {
    //first allocation of the char array
    int size = 100;
    int counter = 0;
    //rewind(fd);
    text->charsArray = (char*)calloc(size, sizeof(char));
    lseek(fd,0,SEEK_SET);
    char c;
    int ans;
    while (1) {
        ans = read(fd, &c, BUFF_SIZE);
        if (ans == 0) //end of file
            break;
        if ((c != ' ') && (c != '\n') && (c != '\t')) { // a char that is not space
            counter++;
            char c2[2] = {0};
            c2[0] = c;
            strcat(text->charsArray,c2);
        }
        if ((size - counter) == 10) { //realloc
            size += ADDITION;
            text->charsArray = realloc(text->charsArray,size * sizeof(char));
        }
    }
}
/**
 * turn string to uppercase
 * @param string to convert
 */
void toUpperCase(char* string) {
    char *s = string;
    while (*s) {
        *s = toupper((unsigned char) *s);
        s++;
    }
}
/**
 * close files
 * @param fd1
 * @param fd2
 */
void closeFiles(int fd1, int fd2) {
    close(fd1);
    close(fd2);
}

int main(int argc, char **argv) {
    if (checkParams(argc))
        return 0;

    int fdin1,fdin2;

    if (!openFiles(argv, &fdin1, &fdin2))
        return 0;

    int finalAnswer = 2;
    if (isFileSame(fdin1,fdin2)) {
        finalAnswer = 1;
    } else {
        Text text1, text2;
        getText(&text1, fdin1);
        getText(&text2, fdin2);
        //turn the char array to uppercase to check if similar
        toUpperCase(text1.charsArray);
        toUpperCase(text2.charsArray);
        if (strcasecmp(text1.charsArray, text2.charsArray) == 0) //check if the two text is similar
            finalAnswer = 3;
        free(text1.charsArray);
        free(text2.charsArray);
    }

    closeFiles(fdin1,fdin2);
    return finalAnswer;
}