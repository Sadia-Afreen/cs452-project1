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

int execute_command(char **argv, struct shell *sh)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0)
  {
    pid_t child = getpid();
    setpgid(child, child);
    tcsetpgrp(sh->shell_terminal, child);

    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);

    if (execvp(argv[0], argv) == -1)
    {
      perror("execvp failed");
    }
    exit(EXIT_FAILURE);
  }
  else if (pid < 0)
  {
    perror("fork failed");
  }
  else
  {

    do
    {
      wpid = waitpid(pid, &status, 0);
      if (WIFSTOPPED(status))
      {
        tcsetpgrp(sh->shell_terminal, sh->shell_pgid);
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));

    tcsetpgrp(sh->shell_terminal, sh->shell_pgid);

    tcgetattr(sh->shell_terminal, &sh->shell_tmodes);
    tcsetattr(sh->shell_terminal, TCSADRAIN, &sh->shell_tmodes);
  }
  return 1;
}

int main(int argc, char **argv)
{

  parse_args(argc, argv);
  struct shell my_shell;
  sh_init(&my_shell);

  char *line;
  using_history();

  while ((line = readline(my_shell.prompt)))
  {
    line = trim_white(line);
    add_history(line);

    char **argv = cmd_parse(line);
    if (do_builtin(&my_shell, argv))
    {
      cmd_free(argv);
      free(line);
    }
    else
    {
      execute_command(argv, &my_shell);
      cmd_free(argv);
      free(line);
    }
  }

  sh_destroy(&my_shell);

  return 0;
}
