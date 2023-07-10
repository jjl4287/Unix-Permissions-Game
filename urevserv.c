/* 
 * urev.c - A program that attempts to recreate all current urev 
 *          features and improve upon many aspects.
 * 
 * Written by - Jakob Langtry
 */
#include <time.h>
#include <regex.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "socklib.c"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

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
	int      onoff; // 1 is request, 0 is giving answer
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
	unsigned long  timeprnt;
} SERVER;

SSTR gsstr(int);
int  gans(int);
int isadir(char *);
void process_rq(CLIENT, unsigned long, int);
void answerr_rq(CLIENT, int);
unsigned long timedelay(CLIENT);

int main (int argc, char **argv)
{
	int sock, fd;
	unsigned long timechk;
	CLIENT request;

	// make server socket
	sock = make_server_socket (31337);
	if (sock == -1)
	{
		printf("error\n");
		exit (2);
	}

	// accept all connections, grab the struct,
	// decide whether they are still timed out, 
	// decide whether its a question or an answer,
	// call functions accordingly
	while (1)
	{
		fd = accept (sock, NULL, NULL);

		printf("got request\n");
		recv(fd, &request, sizeof(CLIENT), 0);
		if (request.onoff == 0)
		{
			printf("accepted answer\n");
			answerr_rq(request, fd);
		}
		else if ((timechk = timedelay(request)) != 0) 
		{
			printf("accepted question\n");
			process_rq(request, timechk, fd);
		}
		else  process_rq(request, 0, fd);
		printf("finished request\n");

	}
	close(sock);
	return (0);
}

/*
 * timedelay - a function that will check if they have
 * 			   answered the questions they are alloted
 * 			   and gives them how much longer they have
 * 			   to wait.
 * 			   
 */
unsigned long timedelay(CLIENT request)
{
	int qnum = 0;
	int q = 12;
	int tnum = 15600;
	FILE *numins;
	FILE *timedelay;
	unsigned long timechk;
	struct stat chkifexists;
	char *numpath = (char *)malloc(30);
	char *timedel = (char *)malloc(25);
	sprintf(numpath, "./%s/numins",    request.login);
	sprintf(timedel, "./%s/timedelay", request.login);
	// check if username directory exists, create it if not
	if (!isadir(request.login)) mkdir(request.login, 0755);

	// check for mini more and t options
	if (request.more == 1)
	{
		tnum = tnum*2;
		q = 24;
	}
	if (request.mini)
	{
		tnum = tnum/2;
		q = 6;
	}
	if (request.tnum != 0)
	{
		tnum -= request.tnum;
	}

	// if numins/timedelay doesnt exist, create it 
	if(stat(numpath, &chkifexists) != 0) 
    {
        numins  = fopen(numpath, "w+");
        fprintf(numins, "0\n");
		fclose(numins);
    }
	if(stat(timedel, &chkifexists) != 0) 
    {
        timedelay  = fopen(timedel, "w+");
        fprintf(timedelay, "100"); 
		fclose(timedelay);
		return(0);
    }

	// scan timedelay and numebr of questions
	timedelay = fopen(timedel, "r+");
	fscanf(timedelay, "%lu", &timechk);
	fclose(timedelay);
	numins  = fopen(numpath, "r+");
	fscanf(numins,"%d", &qnum);
	fclose(numins);

	// if we have done a set of q questions, and the
	// current time is less than the delay time, give thi difference
	// and tell them to screw off
	if((qnum%q == 0)&&(((unsigned long)time(NULL))<timechk))
	{
		timechk = timechk - (unsigned long)time(NULL);
		return(timechk);
	}
	// if none of those things were true, assume normal question
	// update delay and move on
	timedelay = fopen(timedel, "w+");
	fprintf(timedelay, "%lu", (unsigned long)time(NULL) + tnum);
	fclose(timedelay);
	free(numpath);
	free(timedel);
	return(0);
}


/*
 * process_rq - a function that will provide a question to the
 *				client given there is not a current time delay
 */
void process_rq(CLIENT request, unsigned long timechk, int fd)
{
	//int qnum = 0;
	//FILE *numins;
    FILE *curproblem;
	SERVER response;
	char *numpath = (char *)malloc(30);
	char *curpath = (char *)malloc(25);
	sprintf(numpath, "./%s/numins", request.login);
	sprintf(curpath, "./%s/curproblem", request.login);
	// get number of questions solved for four tier
	/* numins  = fopen(numpath, "r+");
	fscanf(numins,"%d", &qnum);
	fclose(numins); */

	if (!timechk)
	{
		// give question and store in curproblem
		curproblem = fopen(curpath, "w");
		//if (qnum > 96)  response.query = gsstr(1); 
		response.query = gsstr(0);
		response.timeprnt = 0;
		fprintf(curproblem, "%lu:%s:", (unsigned long)time(NULL),response.query.modestr);
		fclose(curproblem);
	}
	else
	{
		response.timeprnt = timechk;
	}
	send(fd, &response, sizeof(response), 0);
	free(curpath);
}

/*
 * answerr_rq - a  function  that  will  act upon  the answer 
 *      		given by the client, and do necessary logging
 */
void answerr_rq(CLIENT answer, int fd)
{
	int qnum = 0;

	// create a lot of file strings
	FILE *rproblems;
    FILE *curproblem;
	FILE *wproblems;
	FILE *numins;
	char *numpath = (char *)malloc(30);
	char *ritpath = (char *)malloc(25);
	char *wrnpath = (char *)malloc(25);
	char *curpath = (char *)malloc(25);
	sprintf(ritpath, "./%s/rproblems", answer.login);
	sprintf(curpath, "./%s/curproblem", answer.login);
	sprintf(wrnpath, "./%s/wproblems", answer.login);
	sprintf(numpath, "./%s/numins", answer.login);
	
	curproblem = fopen(curpath, "r+");
	char *firststuff = (char *)malloc(30);
	fscanf(curproblem, "%s", firststuff);
	fclose(curproblem);
	if(answer.rght)
	{
		rproblems = fopen(ritpath, "a+");
		fprintf(rproblems, "%scorrect:%lu\n", firststuff, (unsigned long)time(NULL));
		fclose(rproblems);
	}
	else
	{
		wproblems = fopen(wrnpath, "a+");
		fprintf(wproblems, "%sincorrect:%lu\n", firststuff, (unsigned long)time(NULL));
		fclose(wproblems);
	}

	numins  = fopen(numpath, "r+");
	fscanf(numins,"%d", &qnum);
	fclose(numins);
	qnum++;
	numins = fopen(numpath, "w+");
	printf("%d\n", qnum);
	fprintf(numins, "%d", qnum);
	fclose(numins);
	unlink(curpath);
	free(firststuff);
	free(wrnpath);
	free(ritpath);
	free(curpath);
	free(numpath);
}

// check if something is a dir
int isadir (char *f)
{
	struct stat info;
	return (stat (f, &info) != -1 && S_ISDIR (info.st_mode)); 
}

// generate random number 0-7
int gans(int offset)
{   
    time_t t;
    srand(((unsigned)time(&t))+(offset));
    int ans = (rand())%8;
    return ans;
}

// generathe a random rwxrwxrwx string
SSTR gsstr(int tiernum)
{
    SSTR question;

	strcpy(question.modestr, "-");
    
    for (int i = 0; i<4; i++)
    {
        question.modeoct[i] = gans(i);
    }

	for (int a = 0; a<3; a++)
	{
		if ((question.modeoct[a+1]&4) == 4)
			question.modestr[(a*3)] = 'r';
		else
			question.modestr[(a*3)] = '-';
		if ((question.modeoct[a+1]&2) == 2)
			question.modestr[(a*3)+1] = 'w';
		else
			question.modestr[(a*3)+1] = '-';
		if ((question.modeoct[a+1]&1) == 1)
			question.modestr[(a*3)+2] = 'x';
		else
			question.modestr[(a*3)+2] = '-';
	}
	/*
	* here is 4 tier code
	if (tiernum)
	{
		if ((question.modeoct[0]&4) == 4)
		{
			if (question.modestr[2] == '-')
				question.modestr[2] = 'S';
			else question.modestr[2] = 's';
		}
		if ((question.modeoct[0]&2) == 2)
		{
			if (question.modestr[5] == '-')
				question.modestr[5] = 'S';
			else question.modestr[5] = 's';
		}
		if ((question.modeoct[0]&1) == 1)
		{
			if (question.modestr[8] == '-')
				question.modestr[8] = 'S';
			else question.modestr[8] = 's';
		}
	}*/

    return question;
}