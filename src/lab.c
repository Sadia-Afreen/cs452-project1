#include "/home/sadiaafreen/Documents/BSU/Fall_24/Operating Systems/cs452-project1/src/lab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  const char *default_prompt = "shell> ";

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
