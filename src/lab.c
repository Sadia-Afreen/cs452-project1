#include "../src/lab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <pwd.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <termios.h>
#include <signal.h>
#include <sys/wait.h>

/**
 * @brief Set the shell prompt. This function will attempt to load a prompt
 * from the requested environment variable, if the environment variable is
 * not set a default prompt of "shell>" is returned.  This function calls
 * malloc internally and the caller must free the resulting string.
 *
 * @param env The environment variable
 * @return const char* The prompt
 */
char *get_prompt(const char *env)
{
  char *prompt = NULL;
  const char *default_prompt = "shell>";

  const char *env_prompt = getenv(env);

  if (env_prompt != NULL)
  {
    prompt = malloc(strlen(env_prompt) + 1);
    if (prompt != NULL)
    {
      strcpy(prompt, env_prompt);
    }
  }
  else
  {
    prompt = malloc(strlen(default_prompt) + 1);
    if (prompt != NULL)
    {
      strcpy(prompt, default_prompt);
    }
  }

  return prompt;
}

/**
 * Changes the current working directory of the shell. Uses the linux system
 * call chdir. With no arguments the users home directory is used as the
 * directory to change to.
 *
 * @param dir The directory to change to
 * @return  On success, zero is returned.  On error, -1 is returned, and
 * errno is set to indicate the error.
 */
int change_dir(char **dir)
{
  const char *path;

  if (dir[1] == NULL)
  {
    path = getenv("HOME");
    if (path == NULL)
    {
      struct passwd *pw = getpwuid(getuid());
      if (pw == NULL)
      {
        perror("getpwuid");
        return -1;
      }
      path = pw->pw_dir;
    }
  }
  else
  {
    path = dir[1];
  }
  if (chdir(path) != 0)
  {
    perror("chdir");
    return -1;
  }

  return 0;
}

/**
 * @brief Convert line read from the user into to format that will work with
 * execvp. We limit the number of arguments to ARG_MAX loaded from sysconf.
 * This function allocates memory that must be reclaimed with the cmd_free
 * function.
 *
 * @param line The line to process
 *
 * @return The line read in a format suitable for exec
 */
char **cmd_parse(char const *line)
{
  long arg_max = sysconf(_SC_ARG_MAX); // Get system's ARG_MAX
  if (arg_max == -1)
  {
    arg_max = 4096; // Fallback if sysconf fails
  }

  char **argv = malloc(sizeof(char *) * (arg_max + 1));
  // Allocate memory for a mutable copy of 'line'
  char *mutable_line = strdup(line);
  if (mutable_line == NULL)
  {
    free(argv);
    return NULL; // Handle strdup failure
  }

  char *token = strtok(mutable_line, " \t\n");
  int i = 0;
  while (token != NULL && i < arg_max)
  {
    argv[i] = strdup(token); // Duplicate token
    if (argv[i] == NULL)
    {
      // Free previously allocated memory on failure
      for (int j = 0; j < i; j++)
      {
        free(argv[j]);
      }
      free(argv);
      free(mutable_line);
      return NULL;
    }
    token = strtok(NULL, " \t\n");
    i++;
  }
  argv[i] = NULL;

  free(mutable_line); // Free the mutable copy
  return argv;
}
/**
 * @brief Free the line that was constructed with parse_cmd
 *
 * @param line the line to free
 */
void cmd_free(char **line)
{
  if (line == NULL)
    return;

  for (int i = 0; line[i] != NULL; i++)
  {
    free(line[i]);
  }
  free(line);
}

/**
 * @brief Trim the whitespace from the start and end of a string.
 * For example "   ls -a   " becomes "ls -a". This function modifies
 * the argument line so that all printable chars are moved to the
 * front of the string
 *
 * @param line The line to trim
 * @return The new line with no whitespace
 */
char *trim_white(char *line)
{
  size_t len = 0;
  char *frontp = line;
  char *endp = NULL;

  if (line == NULL)
  {
    return NULL;
  }
  if (line[0] == '\0')
  {
    return line;
  }

  len = strlen(line);
  endp = line + len;

  while (isspace((unsigned char)*frontp))
  {
    ++frontp;
  }
  if (endp != frontp)
  {
    while (isspace((unsigned char)*(--endp)) && endp != frontp)
    {
    }
  }

  if (frontp != line && endp == frontp)
  {
    *(isspace((unsigned char)*endp) ? line : (endp + 1)) = '\0';
  }
  else if (line + len - 1 != endp)
    *(endp + 1) = '\0';

  endp = line;
  if (frontp != line)
  {
    while (*frontp)
    {
      *endp++ = *frontp++;
    }
    *endp = '\0';
  }

  return line;
}

/**
 * @brief Takes an argument list and checks if the first argument is a
 * built in command such as exit, cd, jobs, etc. If the command is a
 * built in command this function will handle the command and then return
 * true. If the first argument is NOT a built in command this function will
 * return false.
 *
 * @param sh The shell
 * @param argv The command to check
 * @return True if the command was a built in command
 */
bool do_builtin(struct shell *sh, char **argv)
{
  if (argv == NULL || argv[0] == NULL)
    return false;

  if (strcmp(argv[0], "exit") == 0)
  {
    sh_destroy(sh);
    exit(0);
  }

  if (strcmp(argv[0], "cd") == 0)
  {
    if (change_dir(argv) == 0)
    {
      return true;
    }
    return false;
  }

  if (strcmp(argv[0], "pwd") == 0)
  {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
      printf("%s\n", cwd);
      return true;
    }
    else
    {
      perror("getcwd");
      return false;
    }
  }

  if (strcmp(argv[0], "history") == 0)
  {
    HIST_ENTRY **hist_list;
    int i;

    hist_list = history_list();
    if (hist_list == NULL)
    {
      printf("No history available.\n");
      return false;
    }

    for (i = 0; hist_list[i] != NULL; i++)
    {
      printf("%d: %s\n", i + 1, hist_list[i]->line);
    }
  }

  if (strcmp(argv[0], "jobs") == 0)
  {
    for (int i = 0; i < sh->num_bg_processes; i++)
    {
      struct bg_process *bgp = &sh->bg_processes[i];
      if (bgp->is_done)
      {
        printf("[%d] Done    %s &\n", bgp->job_id, bgp->command);
      }
      else
      {
        printf("[%d] %d Running %s &\n", bgp->job_id, bgp->pid, bgp->command);
      }
    }
    return true;
  }

  return false;
}

/**
 * @brief Initialize the shell for use. Allocate all data structures
 * Grab control of the terminal and put the shell in its own
 * process group. NOTE: This function will block until the shell is
 * in its own program group. Attaching a debugger will always cause
 * this function to fail because the debugger maintains control of
 * the subprocess it is debugging.
 *
 * @param sh
 */
void sh_init(struct shell *sh)
{
  sh->prompt = get_prompt("MY_PROMPT");

  sh->shell_terminal = STDIN_FILENO;
  sh->shell_is_interactive = isatty(sh->shell_terminal);

  if (sh->shell_is_interactive)
  {
    while (tcgetpgrp(sh->shell_terminal) != (sh->shell_pgid = getpgrp()))
      kill(sh->shell_pgid, SIGTTIN);

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    sh->shell_pgid = getpid();
    if (setpgid(sh->shell_pgid, sh->shell_pgid) < 0)
    {
      perror("Couldn't put the shell in its own process group");
      exit(1);
    }

    tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
    tcgetattr(sh->shell_terminal, &sh->shell_tmodes);
  }
}

/**
 * @brief Destroy shell. Free any allocated memory and resources and exit
 * normally.
 *
 * @param sh
 */
void sh_destroy(struct shell *sh)
{
  if (sh->prompt)
  {
    free(sh->prompt);
  }
}

/**
 * @brief Parse command-line arguments when the shell is launched.
 *
 * @param argc Number of arguments
 * @param argv The argument array
 */
void parse_args(int argc, char **argv)
{
  int opt;
  while ((opt = getopt(argc, argv, "v")) != -1)
  {
    switch (opt)
    {
    case 'v':
      printf("Shell version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
      return 0;
    case '?':
      if (isprint(optopt))
        fprintf(stderr, "Unknown \n", optopt);
      else
        fprintf(stderr, "Unknown \n", optopt);
      return 1;
    default:
      abort();
    }
  }
  return 0;
}
