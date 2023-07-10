/* 
 * urev.c - A program that attempts to recreate all current urev 
 *          features and improve upon many aspects.
 * 
 * 
 * Written by - Jakob Langtry
 */


#include <time.h>
#include <stdio.h>
#include <regex.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>
#include "socklib.c"

#define LEFTEDGE  00
#define RIGHTEDGE 30

/*
 * SSTR - super string which contans question and answer
 */
typedef struct sstr
{
    char modestr[8];
    int  modeoct[4];
} SSTR;

/*
 * CLIENT - a struct which will contain all information 
 *          that the server will need to recieve
 */
typedef struct fromcli
{
	long int chksm;
	char login[10];
	int      onoff; // 1 is request, 0 is giving right
    int       tier;
	int       mini;
	int       more;
	int       tnum;
    int       demo;
    int       rght;
} CLIENT;

/*
 * SERVER - a struct which will contain all needed 
 *          info the  server will  give the client
 */
typedef struct fromsrv
{
	SSTR query;
	int  qnum;
    unsigned long timeprnt;
} SERVER;

void  drwq(char[9]);
void  help();
int   getans(int *);
int   getlineans(int *);

int main(int argc, char **argv)
{
    int  sockfd, sockfdtwo;
    int  line = 0;
    int  rit  = 0;
    int  q    = 12;
    CLIENT request;
    CLIENT answer;
    SERVER response;

    request.tnum = 0;
    int opt     = -1;  // variable to store getopt() return
    while ((opt = getopt(argc, argv, "4dlmMT:hV")) != -1)
    {
        switch (opt)  // sets struct vars for options to send to server
        {
            case '4': // -4 is given
                request.tier = 1;
                break;
            case 'd': // -d is given
                request.demo = 1;
                break;
            case 'l': // -l is given
                line = 1;
                break;
            case 'm': // -m is given
                request.mini = 1;
                q = 6;
                break;
            case 'M': // -M is given
                request.more = 1;
                q = 24;
                break;
            case 'T': // -T is given
                request.tnum = atoi(optarg);
                if ((request.tnum > 37)||(request.tnum < 4))
                {
                    printf("Specify value 4-37\n");
                    exit(2);
                }
                request.tnum = request.tnum*60;
                break;
            case 'h': // -h is given
                help();
                exit(1);
            case 'V': // -V is given
                printf("Author:  Jakob Langtry\n");
                printf("Version: alpha 1.0.0\n");
                exit(1);
            default:  // nothing is given
                break;
        }
    }
    
    // save login information
    getlogin_r(request.login, 9);
    getlogin_r(answer.login, 9);

    // if urev delay is set then send that to the server
    if ((getenv("UREV_DELAY"))&&(request.mini == 1))
    {
        request.tnum += 40*60;
    }
    else if ((getenv("UREV_DELAY"))&&(request.more == 1))
    {
        request.tnum += 160*60;
    }
    else if (getenv("UREV_DELAY"))
    {
        request.tnum += 80*60;
    }

    // ncurses functions
    initscr();
    crmode();
    noecho();
    clear(); 

    move(1,1);
    addstr(request.login);
    move(9,11);
    addstr("u     g     o");

    for(int i=0; i<q; i++)
    {
        // setting the request/answer structs
        request.onoff = 1;
        answer.onoff  = 0;

        // connect to the server and send request and get question
        sockfd = connect_to_server("lab46", 31337);
        send(sockfd, &request, sizeof(request), 0);
        recv(sockfd, &response, sizeof(response), 0);
        close(sockfd);

        // if the time delay is not done then print it out
        if (response.timeprnt)
        {
            endwin();
            printf("Need to wait %lu hrs and %lu mins.\n\n", ((response.timeprnt)/60)/60, response.timeprnt/60%60);
            exit(1);
        }

        // draw the question
        drwq(response.query.modestr);
        if (line) rit = getlineans(response.query.modeoct);
        else      rit = getans(response.query.modeoct);
        //if (request.demo == 0)
        //{
            if (rit == 1)
            {
                answer.rght = 1;
            }
            else
            {
                answer.rght = 0;
            }
            // connect to server and send answer state
            sockfdtwo = connect_to_server("lab46", 31337);
            send(sockfdtwo, &answer, sizeof(answer), 0);
            close(sockfdtwo);
        //}
    }
    endwin();
}

/*
 * drwq - draw the questions
 */
void drwq(char question[9])
{
    int  row = 10;
    int  col = 10;
    int  p = 0;

    for(int b = 0; b<3; b++)
    {
        move(row,col);
        addch(question[p]);
        addch(question[p+1]);
        addch(question[p+2]);
        col += 6;
        p += 3;
    }
    col = 10;
}

/*
 * getans - take input from user, determine if its right
 */
int getans(int *ans)
{
    char *blank = "                              ";
    char inp[1];
    int  row = 10;
    int  col = 10;
    int  chk = 0;
    int  rit = 0;
    int  blk = 0;
    for (int a=0; a<3; a++) 
    {
        move(row+1,col+((a*6)+1));
        inp[0] = wgetch(stdscr);

        if ((inp[0] == 'q')||(inp[0] == 27)) 
        {
            endwin();
            exit(1);
        }
        for (int b = 48; b<56; b++)
        {
            if ((inp[0] != b))
            {
                blk++;
            }
        }
        if (blk == 8)
        {
            a--;
            blk = 0;
            continue;
        }
        blk = 0;
        if (atoi(inp) == ans[a+1]) 
        {
            addch(inp[0]);
            chk++;
        }  
        else addch('X');
    }

    move(row+1,col+20);
    if (chk == 3)
    {
        addstr("CORRECT");
        rit++;
    }
    else 
    { 
        addstr("INCORRECT");
        rit = 0;
    }

    getch();
    move(row+1,col);
    addstr(blank);
    refresh();
    return(rit);
}

/*
 * getlineans - get answers and determine if they are right
 *              but this time, WITH BACKSPACE!!!!!!!!!!!!!!
 */
int getlineans(int *ans)
{
    char *blank = "                              ";
    int  input[5];
    int  row = 11;
    int  col = 11;
    int  chk = 0;
    int  rit = 0;
    int  blk = 0;
    int    a = 1;

    // the difference here between line and not line
    // is that we enable keypad for function keys, enter
    // and backspaces and such
    
    // keypad requires an int for input, which made me change 
    // a few things
    noecho();
    keypad(stdscr, true);
    move(row,col);
    while (1) 
    {
        input[a] = getch();

        if(input[a] == KEY_ENTER || input[a] == 32)
        {
            break;
        }
        if ((input[a] == 'q')||(input[a] == 27)) 
        {
            endwin();
            exit(1);
        }
        if ((input[a] == KEY_BACKSPACE || input[a] == KEY_DC)&&(col > 11))
        {
            col -= 6;
            move(row,col);
            addstr(" ");
            move(row,col);
            a--;
            blk = 0;
            continue;
        }
        for (int b = 48; b<56; b++) 
        {
            if ((input[a] != b))
            {
                blk++;
            }
        }
        if (blk == 8)
        {
            //a--;
            blk = 0;
            continue;
        }
        blk = 0;
        col += 6;
        addch(input[a]);
        move(row,col);
        a++;
    }

    // do not calculate correctness until enter is pressed
    for (int c = 1; c<4; c++)
    {
        if (input[c]-0x30 == ans[c]) 
        {
            chk++;
        }  
    }
    move(row,col+3);
    if (chk == 3)
    {
        addstr("CORRECT");
        rit++;
    }
    else 
    { 
        addstr("INCORRECT");
        rit = 0;
    }

    getch();
    move(11,11);
    addstr(blank);
    refresh();
    return(rit);
}

void help()
{
    printf("This is the urev help page!\n\n\n");
    printf("Command-line arguments:\n");
    printf(" -4        activate 4-tier mode, regardless of progress.\n");
    printf(" -l,       allow backspace; do not evaluate until ENTER is pressed.\n");
    printf(" -M,       for those who can handle more than 12 problems at a time\n");
    printf("           can with this option do twice as many (24) with twice an\n");
    printf("           interval of delay (from time of last session).\n");
    printf("-m,        for those who cannot handle 12 consecutive problems in a\n");
    printf("           session: provide 6 and time delay for half the time.\n");
    printf("-T #       specify a value 4-37 that deducts that number of minutes\n");
    printf("           from the session delay.\n");
    printf("-h,        for basic usage.\n");
    printf("-V         display current version, author information.\n\n");
    printf("Note:      establishing the UREV_DELAY environment variable to a\n");
    printf("           positive boolean state may influence the outcome of\n");
    printf("           the inter-session time interval by an amenable mitigation.\n\n");
    return;
}