#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "/home/sadiaafreen/Documents/BSU/Fall_24/Operating Systems/cs452-project1/src/lab.h"
#include <readline/readline.h>
#include <readline/history.h>

int main(int argc, char **argv)
{
  int c;

  while ((c = getopt(argc, argv, "v:")) != -1)
  {
    switch (c)
    {
    case 'v':
      printf("Shell version: %d.%d\n", lab_VERSION_MAJOR, lab_VERSION_MINOR);
      return 0;
      break;
    case '?':
      if (isprint(optopt))
        fprintf(stderr, "Unknown", optopt);
      else
        fprintf(stderr, "Unknown", optopt);
      return 1;
    default:
      abort();
    }
  }

  struct shell my_shell;
  my_shell.prompt = get_prompt("MY_PROMPT");
  // printf("Prompt: %s\n", my_shell.prompt);

  char *line;
  using_history();
  while ((line = readline(my_shell.prompt)))
  {
    printf("%s\n", line);
    add_history(line);
    free(line);
  }
  free(my_shell.prompt);

  return 0;
}
