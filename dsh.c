/*
 * dsh.c
 *
 *  Created on: Aug 2, 2013
 *  Revised on: 2/22/2025
 *      Author: chiu
 *      Editor: dmeyer
 */
#include "dsh.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>


void trim(char *str) {
    if (!str || !*str) return;

    // Trim leading wspace
    char *start = str;
    while (isspace((unsigned char)*start)) start++;

    // Trim trailing wspace
    char *end = str + strlen(str) - 1;
    while (end >= start && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';

    // Shift to beginning
    if (start != str) memmove(str, start, end - start + 2);
}

void parse(char *input, char **args) {
    int i = 0;
    char *token = strtok(input, " ");
    while (token && i < MAX_PROC-1) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

char *path_resolve(const char *cmd) {
    char *path = getenv("PATH");
    if (!path) return NULL;

    char *path_copy = strdup(path);
    if (!path_copy) return NULL;

    struct stat sb;
    char *full_path = malloc(MAXBUF);
    if (!full_path) {
        free(path_copy);
        return NULL;
    }

    // Check current dir
    snprintf(full_path, MAXBUF, "./%s", cmd);
    if (stat(full_path, &sb) == 0 && sb.st_mode & S_IXUSR) {
        free(path_copy);
        return full_path;
    }

    // Check path dirs
    char *dir = strtok(path_copy, ":");
    while (dir) {
        snprintf(full_path, MAXBUF, "%s/%s", dir, cmd);
        if (stat(full_path, &sb) == 0 && sb.st_mode & S_IXUSR) {
            free(path_copy);
            return full_path;
        }
        dir = strtok(NULL, ":");
    }

    free(path_copy);
    free(full_path);
    return NULL;
}