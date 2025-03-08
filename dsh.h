#define MAXBUF 256
#define MAX_PROC 1024

//Any global variables go below

//Function declarations go below
void trim(char *input);
void parse(char *input, char **args);
char *path_resolve(const char *command);

