#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "/home/sadiaafreen/Documents/BSU/Fall_24/Operating Systems/cs452-project1/src/lab.h"
#include <readline/readline.h>
#include <readline/history.h>

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
      cmd_free(argv);
      free(line);
    }
  }

  sh_destroy(&my_shell);

  return 0;
}
