/*
    File Name: procmon.h
    Author: Josh Leverette
    Summary: This file provides interfaces for process monitoring.
*/

#ifndef __STD_INC__
#define __STD_INC__

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>

#endif

#ifndef __PROCMON__
#define __PROCMON__

typedef struct _conditional
{
    int type; //0 = null, 1 = process starts, 2 = process dies, 3 = process is running
    int subcon; //subconditional type, 0 = the, 1 = any, 2 = all
    char** procs; //the applicable processes
    int nprocs; //number of processes applicable
} conditional;

// typedef struct _action
// {
//     int type; //0 = null (mistake), 1 = kill process, 2 = start process
//     char* proc; //the process applicable
// } action;

typedef struct _rule
{
    conditional condition; //the condition for the execution of the rule
    char* actions; //the actions associated with meeting the conditions of the rule
} rule;

typedef struct _ruleset
{
    rule* rules; //all of the rules that will be checked and applied
    int nrules; //number of rules
} ruleset;

int main(int argc, char** argv); //the root method
char* substr(char* string, int start, int length);
char** breakall(char* str, char* breaker);
void zero(char* buffer, int len);
void updatelist(); //this updates the list of processes for diff
void diff(); //find the difference in running processes from the last check
void run(char* cmd); //run executable "cmd."
void checkcond(); //checks the conditions and applies actions
void loadrules(char* file); //load the rules from the given file

#endif

