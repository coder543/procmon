/*
    File Name: procmon.c
    Author: Josh Leverette
    Summary: The methods contained here should create an application to monitor existing processes and apply rules.
*/

#include "procmon.h"

char** processes; //an array of strings containing the names of all processes currently running
int procn; //proc count
char** procdiff; //an array of strings containing the names of processes who have died or been started
int prdfn; //proc diff count
ruleset rules; //the set of rules by which procmon will act

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("Error: You must provide a rules script.\n");
        printf("Example: procmon ./gamewatch.prm\n");
        return 127;
    }
    loadrules(argv[1]);
    updatelist();
    reupdate: //infinitely update list, until such time as the program closes
    diff();
    checkcond();
    sleep(1);
    goto reupdate;
}

char* substr(char* string, int start, int length)
{
    //function substr: returns the given piece of "string"
    //accepts three arguments
    //string: a string containing another string
    //start: an integer defining where in the string to begin the substring
    //length: an integer defining how long the substring should be
    //returns a string
    char* ret = (char*)malloc(length + 1);
    ret[length] = 0;
    int i = -1;
    while (++i < length)
    {
        ret[i] = string[i + start];
    }
    return ret;
}

char** breakall(char str[], char* breaker)
{
    //function breakall: splits a string, "str", into an unspecified number of strings based on the breakpoint, "breaker"
    //accepts two arguments
    //buffer: a string containing the information needing splitting
    //breaker: a character that identifies where to break a string
    //returns a string array
    char* pch;
    char** base = (char**)malloc(1024 * sizeof(char*));
    int bcount = 0;
    pch = strtok (str,breaker);
    while (pch != NULL)
    {
        base[bcount++] = pch;
        pch = strtok (NULL, breaker);
    }
    base[bcount] = 0;
    return base;
}

void zero(char* buffer, int len)
{
    //function zero: zeroes a string of characters
    //accepts two arguments
    //buffer: a string needing to be zeroed
    //len: an integer telling how long buffer is
    int i = -1;
    while (++i < len)
    {
        buffer[i] = 0;
    }
}


int comparator(const void* elem1, const void* elem2)
{
    char* e1 = *(char**) elem1;
    char* e2 = *(char**) elem2;
    int i = 0;
    int l1 = strlen(e1);
    int l2 = strlen(e2);
    while (e1[i] == e2[i] && i < l1 && i < l2)
        i++;
    return e1[i] - e2[i];
}

void updatelist()
{
    //function updatelist: updates the list of processes
    FILE* fpipe;
    // char* plist = (char*)malloc(16 * 1024); //16kb is not too much to ask for...
    // zero(plist, 16 * 1024);
    if (!(fpipe = (FILE*)popen("ps -e", "r")))
    {   //if fpipe is null after popen-ing
        printf("This is not supposed to happen... you don't have the 'ps' executable installed? [error: ps -e could not be run!]");
        exit(3); //exit with error.
    }
    int i;
    int cl = 0; //current location
    char* buffer = (char*)malloc(128);
    fgets( buffer, 128, fpipe);
    processes = (char**)malloc(sizeof(char*) * 8192);
    while (cl < 8192)
    {
        zero(buffer, 128);
        fgets( buffer, 128, fpipe);
        char** broken = breakall(buffer, " ");
        i = -1;
        while (broken[++i] != 0);
        i--;
        if (i == -1)
            break; //end of the list
        int len = strlen(broken[i]);
        broken[i][len-1] = 0;
        if (!strcmp("<defunct>", broken[i]))
            continue;
        processes[cl] = (char*)malloc(len);
        memcpy(processes[cl], broken[i], len);
        cl++;
    }
    procn = cl;
    qsort(processes, procn, sizeof(char*), comparator);
    // i = -1;
    // while (++i < procn)
    //     printf("%s\n", processes[i]);
}

void diff()
{
    char** old = processes;
    int oldn = procn;
    updatelist();
    procdiff = (char**)malloc(sizeof(char*)*8192);
    int pdc = 0; //procdiff counter
    int maxn = oldn > procn ? oldn : procn;
    int i = -1;
    int j = -1;
    while (++i < oldn && ++j < procn)
    {
        if (strcmp(old[i], processes[j]))
        {
            //we have a difference
            int jt = j;
            while (strcmp(old[i], processes[j++]) && j < procn);
            if (j == procn)
            {
                procdiff[pdc] = (char*)malloc(strlen(old[i]) + 1);
                memcpy(procdiff[pdc], "-", 1);
                memcpy(procdiff[pdc]+1, old[i], strlen(old[i]));
                pdc++;
                j = jt-1;
            }
            else
                j--;
        }
    }
    i = -1;
    j = -1;
    while (++i < oldn && ++j < procn)
    {
        if (strcmp(old[i], processes[j]))
        {
            //we have a difference
            int it = i;
            while (strcmp(old[i++], processes[j]) && i < oldn);
            if (i == oldn)
            {
                procdiff[pdc] = (char*)malloc(strlen(processes[j]) + 1);
                memcpy(procdiff[pdc], "+", 1);
                memcpy(procdiff[pdc]+1, processes[j], strlen(processes[j]));
                pdc++;
                i = it-1;
            }
            else
                i--;
        }
    }
    prdfn = pdc;
    // i = -1;
    // while (++i < pdc)
    //     printf("%s\n", procdiff[i]);
    // printf("%i out of (%i:%i) changed life-state\n", pdc, oldn, procn);
    // printf("----------\n");
}

void run(char* cmd)
{
    pid_t pID = fork();
    if (!pID)
    {
        // int i = -1;
        // int j = 1;
        // int len = strlen(cmd);
        // char* escapedcmd = (char*)malloc(len*2);
        // escapedcmd[0] = '\'';
        // while (++i < len)
        // {
        //     if (cmd[i] == '\'')
        //     {
        //         escapedcmd[j++] = '\\';
        //     }
        //     escapedcmd[j++] = cmd[i];
        // }
        // escapedcmd[j++] = '\'';
        // printf("%s", escapedcmd);
        // system(escapedcmd);
        system(cmd);
        exit(0);
    }
}

void checkcond()
{
    int i = -1;
    while (++i < rules.nrules)
    {
        rule tc = rules.rules[i]; //to check
        int bfinal = 0; //helps with all vs any
        if (tc.condition.subcon == 2)
            bfinal = 1;
        if (tc.condition.type == 0)
        {
            printf("unexpected error\n");
            exit(7);
        }
        else if (tc.condition.type == 3)
        {
            int k = -1;
            while (++k < tc.condition.nprocs)
            {
                int btmp = 0;
                int j = -1;
                while (++j < procn)
                {
                    if (!strcmp(processes[j], tc.condition.procs[k]))
                    {
                        btmp = 1;
                        break;
                    }
                }
                if (tc.condition.subcon == 0 || tc.condition.subcon == 1)
                    bfinal = bfinal || btmp;
                else
                    bfinal = bfinal && btmp;
            }
        }
        else
        {
            if (tc.condition.type == 1)
            {
                int k = -1;
                while (++k < tc.condition.nprocs)
                {
                    int btmp = 0;
                    int j = -1;
                    while (++j < prdfn)
                    {
                        if (procdiff[j][0] == '+')
                        {
                            if (!strcmp(procdiff[j]+1, tc.condition.procs[k]))
                            {
                                btmp = 1;
                                break;
                            }
                        }
                    }
                    if (tc.condition.subcon == 0 || tc.condition.subcon == 1)
                        bfinal = bfinal || btmp;
                    else
                        bfinal = bfinal && btmp;
                }
            }
            else if (tc.condition.type == 2)
            {
                int k = -1;
                while (++k < tc.condition.nprocs)
                {
                    int btmp = 0;
                    int j = -1;
                    while (++j < prdfn)
                    {
                        if (procdiff[j][0] == '-')
                        {
                            if (!strcmp(procdiff[j]+1, tc.condition.procs[k]))
                            {
                                btmp = 1;
                                break;
                            }
                        }
                    }
                    if (tc.condition.subcon == 0 || tc.condition.subcon == 1)
                        bfinal = bfinal || btmp;
                    else
                        bfinal = bfinal && btmp;
                }
            }
            else
            {
                //ERRROR
            }
        }
        //printf("%i: %i\n", i, bfinal);
        if (bfinal)
        {
            run(tc.actions);
        }
    }
    //printf("\n");
}

void loadrules(char* file)
{
    
    FILE* rulefile = fopen(file, "r"); //open the rules file
    if (rulefile == NULL) //check to make sure there are actually rules here
    {
        printf("%s is not a file (or could not be opened for reading)\n", file); //print the error message
        exit(2); //exit with an error
    }
    
    char fc[16384]; //arbitrary 16kb size limit
    zero(fc, 16 * 1024); //zero out the buffer
    int i = -1; //loop counting integer
    while ((fc[++i] = fgetc(rulefile)) != EOF); //read the file into memory
    fc[i] = 1;
    char** rbuff = breakall(fc, "\n");
    
    i = -1;
    int rcnt = 0;
    int ecnt = 0;
    while (rbuff[++i][0] != 1)
    {
        if (rbuff[i][0] == 'i' && rbuff[i][1] == 'f')
            rcnt++;
        if (!strcmp(rbuff[i], "endif"))
            ecnt--;
    }
    fclose(rulefile);
    
    if (rcnt + ecnt != 0)
    {
        if (rcnt + ecnt > 0)
            printf("Syntax error 1, There is an unpaired if statement! Exiting.\n");
        else
            printf("Syntax error 1, There is an unpaired endif statement! Exiting.\n");
        exit(1);
    }

    rules.rules = (rule*)malloc(sizeof(rule)*rcnt);
    rules.nrules = rcnt;

    int inif = 0; //*in*side an *if* statement
    int len = i;
    i = -1;
    int ruleptr = 0;
    int actptr = 0; //actions pointer
    while (++i < len)
    {
        //interpret the rules
        if (rbuff[i][0] == '#')
            continue; //skip comments
        else if (inif)
        {
            if (!strcmp(rbuff[i], "endif")) //if equal
            {
                inif = 0;
                //printf("%s--------\n", rules.rules[ruleptr].actions);
                ruleptr++;
                actptr = 0;
            }
            else
            {
                memcpy(rules.rules[ruleptr].actions + actptr, rbuff[i], strlen(rbuff[i]));
                actptr += strlen(rbuff[i]);
                rules.rules[ruleptr].actions[actptr++] = '\n';
            }
        }
        else if (!strcmp(substr(rbuff[i], 0, 2), "if"))
        {
            inif = 1;
            rules.rules[ruleptr].actions = (char*)malloc(4096);

            char* tmp = rbuff[i];
            int x = 2;
            if (!strcmp(substr(tmp, x, 8), " starts "))
            {
                rules.rules[ruleptr].condition.type = 1;
                x += 8;
            }
            else if (!strcmp(substr(tmp, x, 6), " dies "))
            {
                rules.rules[ruleptr].condition.type = 2;
                x += 6;
            }
            else if (!strcmp(substr(tmp, x, 9), " running "))
            {
                rules.rules[ruleptr].condition.type = 3;
                x += 9;
            }

            if (!strcmp(substr(tmp, x, 4), "the "))
            {
                rules.rules[ruleptr].condition.subcon = 0;
            }
            else if (!strcmp(substr(tmp, x, 4), "any "))
            {
                rules.rules[ruleptr].condition.subcon = 1;
            }
            else if (!strcmp(substr(tmp, x, 4), "all "))
            {
                rules.rules[ruleptr].condition.subcon = 2;
            }
            else
            {
                printf("Syntax error 3 on line %i! Exiting.", i);
                exit(3);
            }
            x += 3;
            char** proctmp = (char**)malloc(sizeof(char*) * 10);
            int ptmpreal = 0; //real number of procs
            int ptc = 0;
            int inq; //in quotes
            while (tmp[++x] != ';')
            {
                if (tmp[x] == '\"')
                {
                    inq = !inq;
                    if (inq)
                        proctmp[ptmpreal] = (char*)malloc(128);
                    else
                    {
                        proctmp[ptmpreal][ptc] = 0;
                        ptc = 0;
                        ptmpreal++;
                    }
                }
                else if (inq)
                    proctmp[ptmpreal][ptc++] = tmp[x];
                else if (tmp[x] != ' ')
                {
                    printf("Syntax error 5 on line %i! Exiting.", i);
                    exit(5);
                }
            }
            if (ptmpreal == 0)
            {
                printf("Syntax error 4 on line %i! Exiting.", i);
                exit(4);
            }
            rules.rules[ruleptr].condition.procs = proctmp;
            rules.rules[ruleptr].condition.nprocs = ptmpreal;
        }
        else
        {
            printf("Syntax error 2 on line %i! Exiting.\n", i);
            exit(2);
        }
    }
}