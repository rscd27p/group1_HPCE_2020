// Compile with: gcc -o memcheck memcheck.c -ldl

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/wait.h>


int main(int argc, char** argv){
    // Load the program to analyze with the LD_PRELOAD variable set to our custom libmemcheck.so
    char *const program_path = "/home/project2/git/group1_HPCE_2020/Homework_1/Memory_leaks/case4";
    char *const args[] = {program_path,NULL};
    char *const envs[] = {"LD_PRELOAD=lib/libmemcheck.so",NULL};
    execve(program_path,args,envs);
    return 0;
}


