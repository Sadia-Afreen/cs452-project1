#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "../src/lab.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <termios.h>
#include <signal.h>

int execute_command(char **argv, struct shell *sh, int background)
{
  if (argv == NULL || argv[0] == NULL)
    return 0;
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0)
  {
    if (!background)
    {
      pid_t child = getpid();
      setpgid(child, child);
      tcsetpgrp(sh->shell_terminal, child);

      signal(SIGINT, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);
      signal(SIGTTIN, SIG_DFL);
      signal(SIGTTOU, SIG_DFL);
    }
    execvp(argv[0], argv);
    exit(EXIT_FAILURE);
  }
  else if (pid < 0)
  {
    perror("fork failed");
  }
  else
  {

    if (background)
    {
      sh->bg_processes[sh->num_bg_processes].job_id = sh->num_bg_processes + 1;
      sh->bg_processes[sh->num_bg_processes].pid = pid;
      sh->bg_processes[sh->num_bg_processes].command = strdup(argv[0]);
      printf("[%d] %d %s\n", sh->bg_processes[sh->num_bg_processes].job_id, pid, argv[0]);
      sh->num_bg_processes++;
    }
    else
    {
      do
      {
        waitpid(pid, &status, WUNTRACED);
        if (WIFSTOPPED(status))
        {
          tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
        }
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));

      tcsetpgrp(sh->shell_terminal, sh->shell_pgid);

      tcgetattr(sh->shell_terminal, &sh->shell_tmodes);
      tcsetattr(sh->shell_terminal, TCSADRAIN, &sh->shell_tmodes);
    }
  }
  return 1;
}

int is_background(char *line)
{
  size_t len = strlen(line);
  while (len > 0 && isspace(line[len - 1]))
    len--;
  if (len > 0 && line[len - 1] == '&')
  {
    line[len - 1] = '\0';
    return 1;
  }
  return 0;
}

int main(int argc, char **argv)
{

  parse_args(argc, argv);
  if (argc > 1 && strcmp(argv[1], "-v") == 0)
  {
    return 0;
  }

  struct shell my_shell = {0};
  sh_init(&my_shell);

  char *line;
  using_history();
  int status;
  while ((line = readline(my_shell.prompt)))
  {
    line = trim_white(line);
    add_history(line);

    int background = is_background(line);
    char **argv = cmd_parse(line);
    if (do_builtin(&my_shell, argv))
    {
      cmd_free(argv);
      free(line);
    }
    else
    {
      execute_command(argv, &my_shell, background);
      cmd_free(argv);
      free(line);
    }

    for (int i = 0; i < my_shell.num_bg_processes; i++)
    {
      pid_t pid = waitpid(my_shell.bg_processes[i].pid, &status, WNOHANG);
      if (pid > 0)
      {
        my_shell.bg_processes[i].is_done = 1;
      }
    }
  }

  sh_destroy(&my_shell);

  return 0;
}
