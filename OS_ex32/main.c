//Omri Sak 205892615
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_PATH 150
#define SYS_ERROR "Error in system call\n"
#define SYS_ERROR_CHARS 21
#define LINE_ERROR "Error in line number\n"
#define LINE_ERROR_CHARS 21
#define BUFF_SIZE 1
#define RESULTS_FILE "results.cvs"
#define STUDENT_RESULT_FILE "StudentResultFile.txt"
#define NO_C_FILE ",0,NO_C_FILE\n"
#define NO_C_FILE_CHARS 13
#define COMPILATION_ERROR ",20,COMPILATION_ERROR\n"
#define COMPILATION_ERROR_CHARS 22
#define TIMEOUT_ERROR ",40,TIMEOUT_ERROR\n"
#define TIMEOUT_ERROR_CHARS 18
#define GREAT_JOB ",100,GREAT_JOB\n"
#define GREAT_JOB_CHARS 15
#define SIMILAR_OUTPUT ",80,SIMILAR_OUTPUT\n"
#define SIMILAR_OUTPUT_CHARS 19
#define BAD_OUTPUT ",60,BAD_OUTPUT\n"
#define BAD_OUTPUT_CHARS 15
#define COMPILE_NAME "omri.out"
#define COMP_NAME "comp.out"

/**
 * struc for the configuration
 */
typedef struct Configuration {
    char directoryPath[MAX_PATH];
    char inputPath[MAX_PATH];
    char correctOutputPath[MAX_PATH];
} Configuration;
/**
 * check if there is enough arguments
 * @param args
 * @return -1 if not 0 if ok
 */
int checkParams(int args) {
    if (args != 2) {
        write(STDERR_FILENO,"Error: program needs one path file\n", 35);
        return -1;
    }
    return 0;
}
/**
 * openfile for read only
 * @param fd file descriptor
 * @param path of the file location
 * @return 1 if open successfully, else return 0
 */
int openFile(int* fd, char* path) {
    if ((*fd = open(path,O_RDONLY)) < 0) {
        write(STDERR_FILENO,SYS_ERROR,SYS_ERROR_CHARS);
        return 0;
    }
    return 1;
}

/**
 * add each line to the right place in the configuration structure
 * @param lineNum 1- directoryPath, 2- inputPath, 3- correctOutputPath
 * @param path string path to add
 * @param conf the configuration structure
 */
void addToConfiguration(int lineNum, char* path,Configuration* conf ) {
    switch (lineNum) {
        case 1:
            strcpy(conf->directoryPath, path);
            break;
        case 2:
            strcpy(conf->inputPath, path);
            break;
        case 3:
            strcpy(conf->correctOutputPath, path);
            break;
        default:
            write(STDERR_FILENO,LINE_ERROR,LINE_ERROR_CHARS);
    }

}

/**
 * get the configuration path from the file
 * @param fd the file descriptor of the configuration file
 * @param conf configuration structure
 */
void getConfigurationPaths(int fd, Configuration* conf) {
    char line[150] = {0};
    lseek(fd, 0, SEEK_SET);
    char c;
    int ans;
    int lineNum = 1;
    while (1) {
        ans = read(fd, &c, BUFF_SIZE);
        if (ans == 0) //end of file
            break;
        if(c == '\n') {
            addToConfiguration(lineNum, line, conf);
            line[0] = '\0';
            lineNum++;
            continue;
        }
        else {
            char c2[2] = {0};
            c2[0] = c;
            strcat(line,c2);
        }
    }
}
/**
 * open the main student directory
 * @param path to the folder
 * @return DIR*
 */
DIR* getMainStudentDirectory(char* path) {
    DIR* pDir;
    if ((pDir = opendir(path)) == NULL) {
        write(STDERR_FILENO,SYS_ERROR,SYS_ERROR_CHARS);
    }
    return pDir;
}
/**
 * cehck if file is c file
 * @param file to be checkd
 * @return 1 if ture else 0
 */
int isCfile(char* file) {
    char* ending = NULL;
    //strrchr- searches for the last occurrence of the character "."
    if((ending = strrchr(file,'.')) != NULL ) {
        if(strcmp(ending,".c") == 0) {
            //ends with .c
            return 1;
        }
    }
    return 0;
}
/**
 * add folder to path
 * @param des path
 * @param src folder
 */
void strcatPath(char* des, char* src) {
    strcat(des, "/");
    strcat(des, src);
}
/**
 * get the directory of the file
 * @param path path of the file, that will be changed to folder path
 */
void getPathOfFile(char *path) {
    char* ending;
    //strrchr- searches for the last occurrence of the character "/"
    if((ending = strrchr(path,'/')) != NULL ) {
        //remove the path file, and replace it with '\0'
        strcpy(ending, "\0");
    }
}

/**
 * search for c file inside the student directory
 * @param path of student directory
 * @param des- the c file name will be save here
 * @return des
 */
char* find_c_file(char *path, char* des) {
    struct dirent* pEntry;
    DIR* pDir;
    pDir = opendir(path);
    if (pDir == NULL) {
        return NULL;
    }
    // loop through all entries in the directories
    while ((pEntry = readdir(pDir))) {
        //jump over the . and .. directories
        if ((strcmp(pEntry->d_name, ".") != 0) && (strcmp(pEntry->d_name, "..") != 0)) {
            if(isCfile(pEntry->d_name)) {
                //found c file, return it
                memcpy(des, pEntry->d_name, strlen(pEntry->d_name) + 1);
                return des;
            } else {
                //create the path for the new folder
                char copyPath[MAX_PATH] = {0};
                strcpy(copyPath, path);
                strcatPath(copyPath,pEntry->d_name);
                //recursively
                char temp[MAX_PATH] = {0};
                find_c_file(copyPath, temp);
                if (temp[0] != '\0') //if not null return, then copy
                    strcpy(des, temp);
                if (des[0] != '\0') { //if the c file was found in inner directory
                    return des; //return the c file
                }
            }

        }
    }
    return NULL;
}
/**
 * this function compile the program using fork and execvp.
 * @param pathFile the file to compile
 */
void compile(char* pathFile) {
    pid_t pid;
    char* command = "gcc";
    char* const argv[]= {"gcc", "-o", COMPILE_NAME, pathFile, NULL};
    pid = fork();
    if (pid == 0) {
        //the son process
        if (execvp(command, argv) == -1) {
            // error occurred
            printf("error in execvp\n");
            write(STDERR_FILENO,SYS_ERROR,SYS_ERROR_CHARS);
        }
    } else {
        //the father status
        int child_status;
        wait(&child_status);
    }
}

/**
 * check if file exist
 * @return 1 if true, 0 eles
 */
int isCompileExist(char* fileName) {
    int fd;
    if ((fd = open(fileName, O_RDONLY)) == -1) {
        usleep(200);
        //file does not exist
        return 0;
    }
    //file exist
    close(fd);
    return 1;
}
/**
 * this program run the student program with the input parameters, and print the result to file
 * @param programName the program name to run
 * @param inputFd the input for the program

 * @return 1 if the program finished before 5 seconds. else return 0
 */
int runUserProgram(char* programName, int inputFd) {
    int studentResult2 = open(STUDENT_RESULT_FILE, O_CREAT | O_RDWR, 0666);
    pid_t pid;
    pid = fork();
    int isFinished = 0;
    char *const args[] = {COMPILE_NAME, NULL};
    int child_status;
    if (pid == 0) {
        //the son process
        dup2(inputFd, STDIN_FILENO); //swap between the file descriptor of the stdIn to inputFd
        dup2(studentResult2, STDOUT_FILENO); //swap between the file descriptor of the stdOutput to studentResultFd
        if (execv(programName, args) == -1) {
            // error occurred
            printf("error in execvp, trying to run student program\n");
            write(STDERR_FILENO,SYS_ERROR,SYS_ERROR_CHARS);
        }
    } else { //the father process
        double seconds = 5;

        while (seconds >= 0) {
            pid_t return_pid = waitpid(pid, &child_status, WNOHANG);
            if (return_pid == pid) {
                //child has finished
                isFinished = 1;
                break;
            }
            usleep(250000);
            seconds -= 0.25;
        }

        if (!isFinished) {
            int status;
            kill(pid, SIGTERM);// gives the child process an opportunity to terminate gracefully
            usleep(100000); //process takes to close down gracefully
            if ( pid != waitpid(pid, &status, WNOHANG)) { //check if child didn't finish
                kill(pid, SIGKILL);
                waitpid(pid, &child_status, 0);
            }
        }

    }
    close(studentResult2);
    return isFinished;
}

/**
 * compare between two file via comp.out
 * @param filePath1 file path to compare
 * @param filePath2 file path to compare
 * @return 1 if the same, 3 if similar, 2 else
 */
int compareOutputs(char* filePath1, char* filePath2) {
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        //child process
        char* const args[] = {COMP_NAME, filePath1, filePath2, NULL};
        if (execv(COMP_NAME, args) == -1) {
            // error occurred
            printf("error in execvp, trying to run student program\n");
            write(STDERR_FILENO,SYS_ERROR,SYS_ERROR_CHARS);
            return 0;
        }
    }
    //father process

    signed int child_status;
    wait(&child_status);
    return WEXITSTATUS(child_status);
}

/**
 * print the correct grade to the cvs file
 * @param res result of the student
 * @param cvsFd the cvs file descriptor, that the grade will be written to
 * @param studentName the name of the student to print to cvs
 */
void writeGrade(int res, int cvsFd, char* studentName) {
    switch (res) {
        case 1: //same files
            write(cvsFd,studentName, strlen(studentName));
            write(cvsFd,GREAT_JOB,GREAT_JOB_CHARS);
            break;
        case 2: //different files
            write(cvsFd,studentName, strlen(studentName));
            write(cvsFd,BAD_OUTPUT,BAD_OUTPUT_CHARS);
            break; //similar files
        case 3:
            write(cvsFd,studentName, strlen(studentName));
            write(cvsFd,SIMILAR_OUTPUT,SIMILAR_OUTPUT_CHARS);
            break;
    }
}

/**
 * calculate the grade of the student and write it in the cvs file
 * @param cvsFd the cvs file
 * @param studentName the name of the student
 * @param pathCfile the path of the c file to compile
 * @param inputPath the input to the c file
 * @param correctOutputFd the correct output to the c file
 * @param pwd current working directory of the comp.out file
 */
void calculateGrade(int cvsFd, char* studentName, char* pathCfile, char* inputPath , char* correctOutputFd, char* pwd) {
    int inputFd;
    if (!openFile(&inputFd,inputPath)) {
        write(STDERR_FILENO,SYS_ERROR,SYS_ERROR_CHARS);
        return;
    }
    //change directory to the path file
    char tempPath[MAX_PATH];
    strcpy(tempPath,pathCfile);
    getPathOfFile(tempPath);
    //change to the file directory, so the compile file will be there
    chdir(tempPath);
    compile(pathCfile);
    if(!isCompileExist(COMPILE_NAME)) { //compile failed
        write(cvsFd,studentName, strlen(studentName));
        write(cvsFd,COMPILATION_ERROR,COMPILATION_ERROR_CHARS);
    } else { //compile successfully
        //create student result file with read amd write permissions
        if (runUserProgram(COMPILE_NAME,inputFd)) {
            //change back to comp.out directory
            chdir(pwd);
            char studentResPath[MAX_PATH] = {0};
            strcpy(studentResPath,tempPath);
            strcatPath(studentResPath,STUDENT_RESULT_FILE);
            int res = compareOutputs(correctOutputFd, studentResPath);
            writeGrade(res, cvsFd, studentName);
        } else {
            //timeout of the student program
            write(cvsFd,studentName, strlen(studentName));
            write(cvsFd,TIMEOUT_ERROR,TIMEOUT_ERROR_CHARS);
        }

    }
    chdir(tempPath);
    unlink(COMPILE_NAME);
    unlink(STUDENT_RESULT_FILE);
    close(inputFd);
}

/**
 * loop through all directories and save grade in file
 * @param pDir the students directory
 * @param path the path of students directory
 * @param configuration - contain all the information about the input and output files
 */
void loopThroughDirectories(DIR* pDir, char* path, Configuration* configuration) {
    struct dirent* pDirent;
    int correctOutputFd;
    char cwd[MAX_PATH];
    getcwd(cwd,MAX_PATH); //save current directory
    //check if open successfully files
    if (!openFile(&correctOutputFd, configuration->correctOutputPath)) {
        write(STDERR_FILENO,SYS_ERROR,SYS_ERROR_CHARS);
        return;
    }
    //create result.cvs file with read amd write permissions
    int cvsFd = open(RESULTS_FILE, O_CREAT | O_RDWR, 0666);
    if (cvsFd < 0) {
        write(STDERR_FILENO,SYS_ERROR,SYS_ERROR_CHARS);
        return;
    }
    char studentPath[MAX_PATH] = {0};
    //read all students directory
    while ((pDirent = readdir(pDir)) != NULL) {
        //jump over the . and .. directories
        if ((strcmp(pDirent->d_name, ".") != 0) && (strcmp(pDirent->d_name, "..") != 0)) {
            printf("Directory: %s\n", pDirent->d_name);
            char cFile[MAX_PATH] = {0};
            strcpy(studentPath, path);
            strcatPath(studentPath,pDirent->d_name); // strcat the full student path
            find_c_file(studentPath, cFile); //get the c file
            if (cFile[0] == '\0') {
                //no file has been found
                printf("No file has been found\n");
                //write to file
                write(cvsFd,pDirent->d_name, strlen(pDirent->d_name));
                write(cvsFd,NO_C_FILE,NO_C_FILE_CHARS);
            } else {
                printf("file %s has been found\n", cFile);
                strcatPath(studentPath,cFile);
                calculateGrade(cvsFd, pDirent->d_name, studentPath, configuration->inputPath,
                        configuration->correctOutputPath, cwd);
            }
        }

    }
    close(cvsFd);
    close(correctOutputFd);
}

int main(int argc, char **argv) {
    Configuration configuration;
    int confFd;

    if (checkParams(argc))
        return 0;
    if(!openFile(&confFd,argv[1]))
        return 0;
    //get configuration paths from file
    getConfigurationPaths(confFd, &configuration);
    close(confFd);
    //open main directory of all students
    DIR* pDir = getMainStudentDirectory(configuration.directoryPath);
    if (pDir == NULL)
        return 0;
    loopThroughDirectories(pDir, configuration.directoryPath, &configuration);
    closedir(pDir);

    return 0;
}