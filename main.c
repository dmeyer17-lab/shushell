/*
 * main.c
 *
 *  Created on: Mar 17 2017
 * 	Revised on: 03/07/2025
 *      Author: david
 * 		Editor: dmeyer
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include "dsh.h"

int handle_cmds(char **args);

int main(int argc, char *argv[])
{

	// DO NOT REMOVE THE BLOCK BELOW (FORK BOMB PREVENTION) //
	struct rlimit limit;
	limit.rlim_cur = MAX_PROC;
	limit.rlim_max = MAX_PROC;
	setrlimit(RLIMIT_NPROC, &limit);
	// DO NOT REMOVE THE BLOCK ABOVE THIS LINE //

	char *cmdline = (char *)malloc(MAXBUF); // stores user input from commmand line

	while (1)
	{
		printf("dsh> ");
		fflush(stdout);

		if (!fgets(cmdline, MAXBUF, stdin))
		{
			printf("\n");
			free(cmdline);
			exit(EXIT_SUCCESS);
		}

		trim(cmdline);

		if (strlen(cmdline) == 0)
			continue;

		char *args[MAX_PROC];
		parse(cmdline, args);

		if (args[0] == NULL)
			continue;

		if (handle_cmds(args))
		{
			continue; // skip fork/exec for handled cmds
		}

		int background = 0; // handling backround proccesses
		if (args[0] && args[1])
		{
			int i = 0;
			while (args[i] != NULL)
				i++;
			if (i > 0 && strcmp(args[i - 1], "&") == 0) // check for '&'
			{
				background = 1;
				args[i - 1] = NULL;
			}
		}

		pid_t pid = fork();

		if (pid == 0) // Child
		{
			if (strchr(args[0], '/')) // Mode 1
			{
				if (access(args[0], X_OK) == 0)
				{
					execv(args[0], args);
					perror("Error: execv");
				}
				else
				{
					fprintf(stderr, "Error: %s: Command not found\n", args[0]);
				}
				_exit(EXIT_FAILURE);
			}
			else // Mode 2
			{
				char *full_path = path_resolve(args[0]);
				if (full_path)
				{
					execv(full_path, args);
					free(full_path);
					perror("Error: execv");
				}
				else
				{
					fprintf(stderr, "Error: %s: Command not found\n", args[0]);
				}
				_exit(EXIT_FAILURE);
			}
		}
		else if (pid > 0) // Parent
		{
			if (!background)
			{
				waitpid(pid, NULL, 0);

				fflush(stdout);
				fflush(stderr);
			}
		}
		else
		{
			perror("fork forked up");
		}
	}
	free(cmdline);
	return EXIT_SUCCESS;
}

int handle_cmds(char **args) // handle specific cmds
{
	if (strcmp(args[0], "exit") == 0)
	{
		printf("Goodbye...\n");
		exit(EXIT_SUCCESS);
	}
	else if (strcmp(args[0], "cd") == 0)
	{
		char *dir = args[1] ? args[1] : getenv("HOME");
		if (!dir)
		{
			fprintf(stderr, "Error: cd HOMEless\n");
		}
		else if (chdir(dir) != 0)
		{
			perror("Error: cd");
		}
		return 1;
	}
	else if (strcmp(args[0], "pwd") == 0)
	{
		char cwd[MAXBUF];
		if (getcwd(cwd, sizeof(cwd)))
		{
			printf("%s\n", cwd);
		}
		else
		{
			perror("Error: pwd");
		}
		return 1;
	}
	return 0;
}