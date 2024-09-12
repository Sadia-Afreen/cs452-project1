#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  printf("hello world\n");
  int c;

  while ((c = getopt (argc, argv, "abc:")) != -1)
  {
    switch (c)
    {
    case 'a':
      printf("get a");
      break;
    case 'b':
      printf("get b");
      break;
    case 'c':
      printf("get c");
      break;
    case '?':
      if (isprint(optopt))
        fprintf(stderr, "Unknown", optopt);
      else
        fprintf(stderr, "Unknown", optopt);
    return 1;
    default:
      abort ();
    }
  }
  
  return 0;
}
