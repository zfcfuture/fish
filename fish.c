#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#define NORMAL	0	// normal command
#define OUT		1	// output redirect
#define IN		2	// input redirect
#define PIPE	3	// pipe command

void	showPrompt();
int		findCMD (char *);
void	getInput(char *);
void	executeCMD(int, char a[][256]);
void	handleInput(char *,int *, char a[][256]);

int main(int argc, char const *argv[])
{
	char	argList[100][256];	// number of args:100;  max size of arg:256B
	char 	*buf = NULL;		// recieve user input
	int		argCount = 0;

	buf = (char *)malloc(256);
	if( buf == NULL ) {
		perror("malloc failed");
		exit(-1);
	}

	while (1)
	{
		memset(buf, 0, 256);	// clear memory
		showPrompt();			// print prompt
		getInput(buf);			// get input
		if (strcmp(buf, "exit\n") == 0)
		{
			break;
		}
		argCount = 0;
		handleInput(buf, &argCount, argList);
		executeCMD(argCount, argList);
	}

	return 0;
}

void showPrompt()
{
	printf("\033[1;33mfish> \033[m");
}

void getInput(char *buf)
{
	int len = 0;
	int ch;

	ch = getchar();
	while (len < 256 && ch != '\n')
	{
		buf[len++] = ch;
		ch = getchar();
	}

	if(len == 256) {
		printf("command is too long \n");
		exit(-1);
	}

	buf[len] = '\n';
	len++;
	buf[len] = '\0';
}

// analytic input args, then put them to argList
void handleInput(char *buf,int *argCount, char argList[100][256])
{
	char	s[2] = " ";
	char	*token;
	int		num = 0;

	token = strtok(buf, s);
	while (token != NULL)
	{
		strcpy(argList[num], token);
		num++;
		// argCount index start from 1
		*argCount = *argCount + 1;
		token = strtok(NULL, s);
	}

	// set argList[last] = NULL
	strcpy(argList[num], "");
	argList[num-1][strlen(argList[num-1]) - 1] = '\0';
}

void executeCMD(int argCount, char argList[100][256])
{
	int		fd;					// file descriptor
	int		del;			// set parm[del]=NULL
	int		flag = 0;			// illegal?
	int		action = 0;			// > < !
	int		backend = 0;		// value(1): &
	char	*file;				// file name
	char	*parm[argCount+1];	// save parameter
	char	*parmNext[argCount+1];	// save parameter

	// extract args from argList
	for (int i = 0; i < argCount; i++)
	{
		parm[i] = (char *) argList[i];
	}
	parm[argCount] = NULL;

	// check for &
	for (int i = 0; i < argCount; i++)
	{
		if (strncmp(parm[i], "&", 1) == 0) {
			if (i == argCount - 1) {
				backend = 1;
				parm[argCount-1] = NULL;
				break;
			} else {
				printf("wrong command\n");
				return;
			}
		}
	}

	// match the type of command
	for (int i = 0; parm[i]!=NULL; i++)
	{
		if (strcmp(parm[i], ">") == 0) {
			flag++;
			action = OUT;
			file = parm[i+1];
			del = i;
			if (parm[i+1] == NULL) flag++;
		}
		if (strcmp(parm[i], "<") == 0) {
			flag++;
			action = IN;
			file = parm[i+1];
			del = i;
			if (i == 0) flag++;
		}
		if (strcmp(parm[i], "|") == 0) {
			flag++;
			action = PIPE;
			del = i;
			int j;
			for (j = i+1; parm[j] != NULL; j++) {
				parmNext[j-i-1] = parm[j];
			}
			parmNext[j-i-1] = parm[j];
			if (parm[i+1] == NULL || i == 0) flag++;
		}
	}

	if (flag > 1) {
		printf("wrong command\n");
		return;
	}

	// fork child process to execute command
	int rc = fork();
	if (rc < 0) {
		printf("fork error\n");
		return;
	} else if (rc == 0)
	{
		switch (action)
		{
		case 0:
			/* child process: normal cmd */
			if (!findCMD(parm[0])) {
				printf("%s: command not found!\n", parm[0]);
				exit(0);	// exit child process
			}
			execvp(parm[0], parm);
			exit(0);
			break;

		case 1:
			/* child process: out-redirect cmd */
			if (!findCMD(parm[0])) {
				printf("%s: command not found!\n", parm[0]);
				exit(0);	// exit child process
			}

			fd = open(file, O_RDWR | O_CREAT | O_TRUNC, 0644);
			// printf("fd= %d\n", fileno(stdout));
			parm[del] = NULL;
			dup2(fd, fileno(stdout));
			execvp(parm[0], parm);
			exit(0);

			break;

		case 2:
			/* child process: in-redirect cmd */
			if (!findCMD(parm[0])) {
				printf("%s: command not found!\n", parm[0]);
				exit(0);	// exit child process
			}

			fd = open(file, O_RDONLY);
			parm[del] = NULL;
			dup2(fd, fileno(stdin));
			execvp(parm[0], parm);
			exit(0);

			break;

		case 3:
			/* child process: pipe cmd */
			if (rc == 0) {
				int fd2;
				int rc2 = fork();

				if (rc2 < 0) {
					printf("fork2 error\n");
					return;
				} else if (rc2 == 0)
				{
					if (!findCMD(parm[0])) {
						printf("%s: command not found!\n", parm[0]);
						exit(0);
					}
					parm[del] = NULL;
					fd2 = open("/tmp/youdonotknowfile", O_WRONLY|O_CREAT|O_TRUNC,0644);
					dup2(fd2, fileno(stdout));
					execvp(parm[0], parm);
					exit(0);
				}

				if (waitpid (rc2, NULL, 0) == -1)
					printf("child process error\n");

				if (!findCMD(parmNext[0])) {
					printf("%s: command not found!\n", parmNext[0]);
					exit(0);
				}
				fd2 = open("/tmp/youdonotknowfile", O_RDONLY);
				dup2(fd2, fileno(stdin));
				execvp(parmNext[0], parmNext);
				exit(0);
			}

			break;

		default:
			break;
		}
	}

	if ( backend == 1 ) {
		printf("[process id %d]\n", rc);
		return ;
	}

	if (waitpid (rc, NULL, 0) == -1)
		printf("child process error\n");
}

int findCMD (char *command)
{
	DIR*             dp;
	struct dirent*   dirp;
	char*			 path[] = { "./", "/bin", "/usr/bin", NULL};

	/* run program in the current dir */
	if( strncmp(command, "./", 2) == 0 ) {
		// ./fork => fork
		command = command + 2;
	}

	/* find executable program [cur, /bin, /usr/bin] */
	int i = 0;
	while (path[i] != NULL) {
		if ( (dp = opendir(path[i]) ) == NULL)
			printf ("can not open /bin \n");
		while ( (dirp = readdir(dp)) != NULL) {
			if (strcmp(dirp->d_name, command) == 0) {
				closedir(dp);
				return 1;
			}
		}
		closedir (dp);
		i++;
	}
	return 0;
}