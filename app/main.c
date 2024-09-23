#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "../src/lab.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <termios.h>

int execute_command(char **argv)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0)
  {
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
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
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
      execute_command(argv);
      cmd_free(argv);
      free(line);
    }
  }

  sh_destroy(&my_shell);

  return 0;
}
