
/**************************************************************************************
Title: This project was so that I could find out who was spending the most time on the school server.
Author: Tenzing Nyima
Created on : November 27 2023
Description: Create a program that print one line for ea h username argument, containing the total time
that the user has spent logged into teh system since record-keeping. Otherwise each user to be reported, it displays
username followed by total login time, accurate to the second.

Purpose: Demostarte system programming ability
Buid with: gcc -o last.c last_

******************************************************************************************/



#define _GNU_SOURCE
#define _XOPEN_SOURCE 500
#ifndef ERROR_EXITS_H
#define ERROR_EXITS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utmpx.h>
#include <pwd.h>
#include<time.h>

#include<sys/types.h>
#include<string.h>
#include <locale.h>
#include <errno.h>
#include<paths.h>
#include<fcntl.h>
#include<lastlog.h>
#define MAX_USERS       100000
#define MAXLEN          128 /* Maximum size of mesage string */
#define BAD_FORMAT_ERROR -1
#define LOCALE_ERROR    -3

/* Some systems define a record type of SHUTDOWN_TIME. If its not defined define it. */
#ifndef SHUTDOWN_TIME
        #define SHUTDOWN_TIME 32 /*Give it a value larger than the other types */
#endif


#define TRUE 1
#define FALSE 0


struct utmp_list{
        struct utmpx ut;
        struct utmp_list *next;
        struct utmp_list *prev;

};
typedef struct utmp_list utlist;

void error_mssge(int code, const char* message);
void fatal_error(int errnum, const char* message);
void usage_error(const char* message);

int get_prev_utrec(int fd, struct utmpx *ut, int *finished)
{
        static off_t saved_offset; /* Where this callis bout to read */
        static int is_first = TRUE;
        static int utsize = sizeof(struct utmpx);
        ssize_t nbytes_read; /* Number of bytes read */

        /* Check if this is the first time it is called. If so , move the file
         * offset to teh last record in teh file and save it in saved offset */
        if (is_first){
                errno =0;
                /* Move to utsize bytes before end of file */
                saved_offset = lseek(fd, -utsize, SEEK_END);
                if(-1 == saved_offset){
                        error_mssge(1, "error tyring to move offset to alst rec of file");
                        return FALSE;
                }
                is_first = FALSE; /* turn off flag*/
        }
        *finished = FALSE; /* Assume we're not done yet */
        if (saved_offset < 0) {
                *finished = TRUE; /* saved_offset < implies we've read entire file */
                return FALSE;
        }
        /* File offset is at teh correct place to read */
        errno = 0;
        nbytes_read = read(fd, ut, utsize);
        if (-1 == nbytes_read){
                /* read() error occured; do not exit -let mein() do that */
                error_mssge(errno, "read");
                return FALSE;
        }
        else if (nbytes_read < utsize){
                /* Full utmpx struct not read; do not exit -let main() do taht */
                error_mssge(2, "less than full record read");
                return FALSE;
        }else{
                /* Successful read of utmpx record */
                saved_offset = saved_offset - utsize; /* Reposition saved_offset */
                if (saved_offset >= 0){
                        /* Seek to preceding record to set up next read */
                        errno = 0;
                        if(-1 ==lseek(fd, -(2*utsize), SEEK_CUR) )
                                fatal_error(errno, "lseek()");
                }
                return TRUE;
        }

}

typedef struct HashEntry{
        char* username;
        time_t seconds;
        struct HashEntry* next;
} HashEntry;

HashEntry* user_time_entries[MAX_USERS];
typedef struct LineTime{
        char *line;
        time_t seconds;
        struct LineTime* next;
}LineTime;
LineTime* line_time_entries[MAX_USERS];

unsigned int hashTable(const char *username){
        unsigned int hash = 0;
        while(*username){
                hash = hash * 31 + *(username++);
        }
        return hash % MAX_USERS;
}
unsigned int hashLine(const char *line){
        unsigned int hash = 0;
        while(*line){
                hash = hash * 31 + *(line++);
        }
        return hash % MAX_USERS;
}


void PrintKeyValue(){
        HashEntry *curr;
        for (int i = 0; i< MAX_USERS; i++){
                curr = user_time_entries[i];
                while( curr != NULL){
                        time_t total_time = curr->seconds;
                        char* curr_user = curr->username;
                        int days = total_time/86400;
                        int hours = (total_time%86400)/3600;
                        int minutes =(total_time % 3600) / 60;
                        int secs = total_time % 60;

                        //time_diff_str[0] = '\0';
                        printf("%s ", curr_user);
                        if (days > 0)
                                printf("%1d day%s ", days, days > 1 ? "s" : "");
                        //HOurs
                        if (hours > 0)
                                printf("%1d hour%s ", hours, hours != 1 ? "s" : "");
                        if (minutes > 0)
                                printf("%1d min%s ", minutes, minutes != 1 ? "s":"");
                        if (secs > 0)
                                printf("%1d sec%s ", secs, secs != 1 ? "s" : "");
                        printf("\n");
                        curr = curr->next;
                }
        }
}


void AddSecondstoHash(struct utmpx *ut, time_t total_time ){
        //unsigned int index = hash(username);
        char line[__UT_LINESIZE + 1];
        strncpy(line, ut->ut_line, __UT_LINESIZE);
        const char *username = ut->ut_user;
        //const char *userline = ut->ut_line;
        time_t seconds = (ut->ut_tv).tv_sec;
        unsigned int index = hashLine(line);
        LineTime* entry = line_time_entries[index];
        line[__UT_LINESIZE] = '\0';

        //printf("Adding Line: %s, Seconds: %ld\n", userline, seconds);
        while(entry != NULL){
                //printf("Entry Name: %s",entry->username);
                if (strcmp(entry->line, line) == 0){
                        //username found and update seconds.
                //      printf("EntryName to add: %s\n", entry->username);
                        entry->seconds += total_time;
                        return;
                }
                entry = entry->next;
        }
        // If username not found , create a new entry
        LineTime *new_line = malloc(sizeof(LineTime));
        new_line->line = strdup(line);
        //newUser->username = strdup(temp_user);
        new_line->seconds = total_time;
        new_line->next = line_time_entries[index];
        line_time_entries[index] = new_line;
}

void addOrUpdateUser(const char *username, time_t seconds){
        unsigned int index = hashLine(username);
        HashEntry *userEntry = user_time_entries[index];
        while(userEntry != NULL){
                if(strcmp(userEntry->username, username) == 0){
                        userEntry->seconds += seconds;
                        return;
                }
                userEntry = userEntry->next;
        }
        HashEntry *newEntry = malloc(sizeof(HashEntry));
        newEntry->username = strdup(username);
        newEntry->seconds = seconds;
        newEntry->next = user_time_entries[index];
        user_time_entries[index] = newEntry;
}
void SubtractSecondstoHash(struct utmpx *ut){
        char line[__UT_LINESIZE +1];
        strncpy(line, ut->ut_line, __UT_LINESIZE);
        line[__UT_LINESIZE] = '\0';

        const char *username = ut->ut_user;
        time_t seconds = (ut->ut_tv).tv_sec;
        //const char *line = ut->ut_line;
        unsigned int index = hashLine(line);
        LineTime *entry = line_time_entries[index];
        LineTime *prev = NULL;

        //char temp_user[__UT_NAMESIZE + 1];
        //strncpy(temp_user, username, __UT_NAMESIZE);
        //temp_user[__UT_NAMESIZE] = '\0';

        while(entry != NULL){
                if (strcmp(entry->line, line)== 0){
                        if (entry->seconds > seconds){
                                entry->seconds -= seconds;
                                addOrUpdateUser(username, entry->seconds);
                                if (prev == NULL){
                                        line_time_entries[index] = entry->next;
                                }else{
                                        prev->next = entry->next;
                                }
                                free(entry);
                        }else{
                                entry->seconds = 0;
                        }
                        return;
                }
                prev = entry;
                entry = entry->next;
        }



}

void format_time_diff(struct utmpx *ut, time_t start_time, time_t end_time, char* time_diff_str){
        time_t total_time;
        time_t secs;
        int minutes;
        int hours;
        int days;

        total_time = end_time -start_time;
        // add total time to hash map of user and hash map.
        days = total_time/86400;
        hours = (total_time % 86400) /3600;
        minutes = (total_time % 3600) /60;
        secs = total_time % 60;
        time_diff_str[0] = '\0';
        //AddSecondstoHash(ut, total_time);
        /* if days > 0 then use a different format */
        if (days > 0)
                sprintf(time_diff_str +strlen(time_diff_str), "%1d day%s", days, days > 1 ? "s": "");

         // Hours
        if (hours > 0) {
                sprintf(time_diff_str + strlen(time_diff_str), "%1d hour%s ", hours, hours != 1 ? "s" : "" );
        }

        // Minutes if minutes is greater than 0 mins
        if (minutes > 0 ){
                sprintf(time_diff_str + strlen(time_diff_str), "%1d min%s", minutes, minutes != 1 ? "s":"");
        }

        // Seconds, if seconds is greater than 0
        if (secs > 0){
                sprintf(time_diff_str + strlen(time_diff_str), "%1ld sec%s", secs, secs != 1 ? "s" :"");
        }


}


void print_one_line(struct utmpx *ut, time_t end_time){
        time_t utrec_time;
        struct tm *bd_end_time;
        struct tm *bd_ut_time;
        char    formatted_login[MAXLEN]; /* String storing formatted login date */
        char    formatted_logout[MAXLEN]; /* String storing formatted logout date */
        char    duration[MAXLEN];       /* String representing session length */
        char *start_date_fmt = "%a %b %d %H:%M";
        char *end_date_fmt = "%H:%M";
        utrec_time = (ut->ut_tv).tv_sec; /* get login time, in seconds */
        SubtractSecondstoHash(ut);
        /* If the end time is 0 or -1, print the appropriate string instead of a time */
        if (ut->ut_time == BOOT_TIME && end_time == 0){
                //format_time_diff(ut, utrec_time, end_time, duration);
                sprintf(duration, "still runnign");
        }
        else if (ut->ut_type == USER_PROCESS && end_time == 0){
                //format_time_diff(ut, utrec_time, end_time,duration);
                sprintf(duration, "still logged in");

        }
        else if (ut->ut_type == USER_PROCESS && end_time == -1){
                //format_time_diff(ut, utrec_time, end_time, duration);
                sprintf(duration, "gone - no logout");
        }
        else /* calcuate and format duration of the session */
                format_time_diff(ut, utrec_time, end_time, duration);

        bd_ut_time = localtime(&utrec_time); /* Convert login time to brokendown time */
        if (bd_ut_time == NULL)
                fatal_error(EOVERFLOW, "localtime");

        if ( 0 == strftime(formatted_login, sizeof(formatted_login), start_date_fmt, bd_ut_time) )
                fatal_error(BAD_FORMAT_ERROR, "Converstion to a date-time string failed or produced " " an empty string \n");
        bd_end_time= localtime(&end_time); /* Convert end time to broken-down time */

        if (bd_end_time == NULL)
                fatal_error(EOVERFLOW, "localtime");

        if( 0 == strftime(formatted_logout, sizeof(formatted_logout), end_date_fmt, bd_end_time) )
                fatal_error(BAD_FORMAT_ERROR, "Converstion to a date-time string failed or produced " " an empty string \n");
        /* Add terminating null byte to host name, otherwise it will print too long. */
        ut->ut_host[sizeof(ut->ut_host)] = '\0';

        /* Print the whole line */
//      printf("%-8.8s %-12.12s  %s\n", ut->ut_user, ut->ut_line, duration);



}


/* Type definitino for the linked list of utmpx recrods. */


void save_ut_to_list(struct utmpx *ut, utlist **list){
        //Where do we save in the list.
        //
        utlist* utmp_node_ptr;
        //printf("save to list user: %s\n", ut->ut_user);
//      AddSecondstoHash( ut);

        /* Allocate a new list node */
        errno = 0;
        if (NULL == (utmp_node_ptr = (utlist*) malloc(sizeof(utlist))))
                fatal_error(errno, "malloc");

        /* Copy teh utmpx record into teh new node */
        memcpy(&(utmp_node_ptr->ut), ut, sizeof(struct utmpx));

        /* Attached the node to teh front of the lsit */
        utmp_node_ptr->next = *list;
        utmp_node_ptr->prev = NULL;
        if (NULL != *list)
                (*list)->prev = utmp_node_ptr;
        (*list) = utmp_node_ptr;
        //printf("save list to user : %s\n", ut->ut_user);

        /* Add seconds to hash[username] */
        AddSecondstoHash(ut, ut->ut_tv.tv_sec);

}

void delete_utnode(utlist *p, utlist **list){

        //SubtractSecondstoHash(p->ut_user, p->ut_tv.tv_sec);

        if (NULL != p->next)
                p->next->prev = p->prev;
        if (NULL != p->prev)
                p->prev->next = p->next;
        else
                *list = p->next;
        free(p);


}
void erase_utlist(utlist **list){
        utlist *ptr = *list;
        utlist *next;
        while(NULL != ptr){
                next = ptr->next;
                free(ptr);
                ptr = next;
        }
        *list = NULL;

}

void fatal_error(int errnum,const char* message){
        error_mssge(errnum, message);
        exit(EXIT_FAILURE);
}


void error_mssge(int code, const char* message){
        if(code > 0)
                fprintf(stderr, "%s\n", strerror(code));
        else
                fprintf(stderr, "error %d: %s\n", code, message);
}
void usage_error(const char *message){
        fprintf(stderr, "usage: %s\n", message);
        exit(EXIT_FAILURE);
}


int main(int argc, char* argv[]){
        struct utmpx utmp_entry;        /* Read info here */
        size_t utsize = sizeof(struct utmpx);
        int fd_utmp; /* read from this descriptro */
        time_t last_boot_time;  /* Time of last boot or reboot */
        time_t last_shutdown_time  =0; /* Time of last shutdown */
        time_t start_time;              /* When wtmp processing started */
        struct tm *bd_start_time;       /* Broken down time represeantation */
        char wtmp_start_str[MAXLEN] ;   /* String to store start time */
        utlist *saved_ut_recs = NULL;   /* an initially empty list */
        char options[] = ":xaf:";               /* getopt string */
        int show_sys_events = FALSE;    /* Flag to indicate -x found */
        int show_all_users = FALSE;
        char usage_msg[MAXLEN];         /* For error messages */
        char *wtmp_file = _PATH_WTMP;
        int done = FALSE;
        int found = FALSE;
        char ch;
        utlist  *p, *next;

        if (( fd_utmp = open(WTMPX_FILE, O_RDONLY)) == -1){
                fatal_error(errno, "while opening" WTMPX_FILE);
        }

        /* check options */
        opterr = 0; /* Turn off error messages by getopt().*/
        while ((ch = getopt(argc, argv, options)) != -1){
                /* call getopt, passing argc and argv and the options string. */
                switch (ch) {
                case 'x':
                        show_sys_events = TRUE;
                        break;
                case 'a':
                        show_all_users = TRUE;
                        break;
                case 'f':
                        printf("optarg: %s", optarg);
                        wtmp_file = optarg;
                        break;
                case '?':
                case ':':
                        fprintf(stderr, "Found invalid option %c\n", optopt);
                        sprintf(usage_msg, "%s [ -x ] [-a] [-f <file>]", basename(argv[0]));
                        usage_error(usage_msg);
                        break;
                }
        }

        /* Set the locale . */
        char *mylocale;
        if ( (mylocale = setlocale(LC_TIME, "") ) == NULL)
                fatal_error( LOCALE_ERROR, "setlocale() could not set the given local");

        /* read the first structure in teh file to capture the time of the first entry. */
        errno = 0;
        if (read(fd_utmp, &utmp_entry, utsize) != utsize){
                fatal_error(errno, "read");
        }

        /* Get the starting time from the first record in wtmp */
        start_time = utmp_entry.ut_tv.tv_sec;

   /* Process the wtmp file */
        while (!done ){
                errno = 0;
                if (get_prev_utrec(fd_utmp, &utmp_entry, &done) ){
                        /* What type of record is this ? For ordinary user logins,
                         * the ut_type field will be USER_PROCESS but for shutdown events, there is no SHUTDOWN_TIME.
                         * We identify a shutdown record by ut_line == ~ and the ut_user == "shutdown". */
                        if ((strncmp(utmp_entry.ut_line, "~", 1) == 0 )&&
                            (strncmp(utmp_entry.ut_user, "shutdown", 8) == 0)){
                                utmp_entry.ut_type = SHUTDOWN_TIME;
                                sprintf(utmp_entry.ut_line, "system down");
                        }


                        switch(utmp_entry.ut_type){
                        case BOOT_TIME:
                                strcpy(utmp_entry.ut_line, "system boot");
                                print_one_line(&utmp_entry, last_shutdown_time);
                                last_boot_time = utmp_entry.ut_tv.tv_sec;
                                if (saved_ut_recs != NULL)
                                erase_utlist(&saved_ut_recs);
                                break;
                        case RUN_LVL:
                                break;
                        case SHUTDOWN_TIME:
                                last_shutdown_time = utmp_entry.ut_tv.tv_sec;
                                if (show_sys_events)
                                        print_one_line(&utmp_entry, last_boot_time);
                                if (saved_ut_recs != NULL)
                                        erase_utlist(&saved_ut_recs);
                                break;
                        case USER_PROCESS:
                                /* Find the logout entry for this login in the saved_ut_recs list.
                                * This should be the entry closest tothe front of the list with teh same ut_line field. */
                                found = 0;
                                p = saved_ut_recs; /* start at beginning */
                                while (NULL != p){
                                        next = p->next;
                                        if ( 0 == (strncmp(p->ut.ut_line, utmp_entry.ut_line, sizeof(utmp_entry.ut_line) ))){
                                                /* The saved node's ut_line matches the one we jsut found */
                                                //SubtractSecondstoHash(&utmp_entry);
                                                //printf("I am getting saved \n");
                                                print_one_line(&utmp_entry, p->ut.ut_tv.tv_sec);
                                                //SubtractSecondstoHash(&utmp_entry);
                                                found = 1;
                                                delete_utnode(p, &saved_ut_recs); /* Delete the node */
                                        }
                                        p = next;
                                }
                                if (!found ){
                                        /* No logout record found for this login.
                                        * If the system was not shut down after this login,
                                        * there is no record because the user is still logged in.
                                        * If the system was shutdown at any time after this login,
                                        * the user cannot still be logged into the same session.
                                        * If there is no saved logout record, it implies that
                                        * the session ended in an abnormal way and the output should
                                        * just indicate teh seur is "gone".
                                        * */
                                        if( last_shutdown_time > 0)
                                                print_one_line(&utmp_entry, (time_t) -1);
                                        else
                                                print_one_line(&utmp_entry, (time_t) 0);
                                }
                                break;
                        case DEAD_PROCESS:
                                /* Create a node in teh saved_ut_recs list for this entry,
                                * provided that the ut_line field is not null */
                                if (utmp_entry.ut_line[0] == 0)
                                        /* There is no line in the entry , so skip it */
                                        continue;
                                else{
                                        //AddSecondstoHash(&utmp_entry);
                                        //printf("Before saving %s\n",utmp_entry.ut_user);
                                        save_ut_to_list(&utmp_entry, &saved_ut_recs);
                                }
                                break;
                        case OLD_TIME: /*Not handled */
                        case NEW_TIME: /* Not handled */
                        case INIT_PROCESS: /* nOT HANDLED */
                        case LOGIN_PROCESS: /* Not handled */
                                break;
                        }/* end of swithch */
                }
                else /* get_prev_utrec() did not read correctly */
                        if (!done)
                                fatal_error(2, " read failed");
        }
                erase_utlist(&saved_ut_recs);
                close(fd_utmp);

                bd_start_time = localtime(&start_time); /* Convert to brokendown time */
                if (0 == strftime(wtmp_start_str, sizeof(wtmp_start_str), "%a %b %d %H:%M:%S %Y", bd_start_time ))
                        fatal_error(BAD_FORMAT_ERROR, "Conversion to a date-time " "string failed or produced an emptty string \n");
                printf("\nwtmpbegins %s\n", wtmp_start_str);
                PrintKeyValue();


                return 0;


}
#endif /* ERROR_EXITS_H */
                            


