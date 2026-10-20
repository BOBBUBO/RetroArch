#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
   const char *home = getenv("HOME");
   char config[4096];

   (void)argc;

   /* Click profiles cannot execute /usr/bin/env; set the config dir here. */
   if (home && *home)
   {
      static const char suffix[] = "/.config/com.libretro.retroarch";
      size_t len = strlen(home);

      if (len + sizeof(suffix) <= sizeof(config))
      {
         memcpy(config, home, len);
         memcpy(config + len, suffix, sizeof(suffix));
         setenv("XDG_CONFIG_HOME", config, 1);
      }
   }

   argv[0] = "retroarch";
   execvp("retroarch", argv);
   perror("retroarch");

   return 1;
}
