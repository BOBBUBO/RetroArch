#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PATH_LEN 4096

struct setting_fix
{
   const char *key;
   const char *from;
   const char *to;
};

/* Settings that cannot work on Ubuntu Touch: its compositor exposes no
 * EGL, so no GL context can be created, and the click AppArmor policy
 * denies /dev/input, so evdev devices are out of reach. */
static const struct setting_fix setting_fixes[] =
{
   { "video_driver", "gl",     "sdl2" },
   { "video_driver", "glcore", "sdl2" },
   { "input_driver", "udev",   "sdl2" },
   { NULL, NULL, NULL }
};

static void join_path(char *dst, size_t size, const char *dir, const char *name)
{
   size_t len      = strlen(dir);
   size_t name_len = strlen(name);

   if (len + name_len + 2 > size)
   {
      dst[0] = '\0';
      return;
   }

   memcpy(dst, dir, len);
   dst[len] = '/';
   memcpy(dst + len + 1, name, name_len + 1);
}

static const struct setting_fix *find_fix(const char *line)
{
   int i;

   for (i = 0; setting_fixes[i].key; i++)
   {
      size_t key_len  = strlen(setting_fixes[i].key);
      size_t from_len = strlen(setting_fixes[i].from);

      if (strncmp(line, setting_fixes[i].key, key_len) != 0)
         continue;
      if (strncmp(line + key_len, " = \"", 4) != 0)
         continue;
      if (strncmp(line + key_len + 4, setting_fixes[i].from, from_len) != 0)
         continue;
      if (line[key_len + 4 + from_len] != '"')
         continue;

      return &setting_fixes[i];
   }

   return NULL;
}

/* Rewrite the settings a previous install left behind that cannot work
 * here, so the app does not keep failing on its old configuration. */
static void fix_config(const char *config_dir)
{
   char path[MAX_PATH_LEN];
   char tmp_path[MAX_PATH_LEN];
   FILE *in;
   FILE *out;
   char line[1024];
   int changed = 0;

   join_path(path, sizeof(path), config_dir, "retroarch/retroarch.cfg");
   join_path(tmp_path, sizeof(tmp_path), config_dir,
         "retroarch/retroarch.cfg.tmp");

   if (!path[0] || !tmp_path[0])
      return;

   in = fopen(path, "r");
   if (!in)
      return;

   out = fopen(tmp_path, "w");
   if (!out)
   {
      fclose(in);
      return;
   }

   while (fgets(line, sizeof(line), in))
   {
      const struct setting_fix *fix = find_fix(line);

      if (fix)
      {
         fprintf(out, "%s = \"%s\"\n", fix->key, fix->to);
         changed = 1;
      }
      else
         fputs(line, out);
   }

   fclose(in);
   fclose(out);

   if (changed)
      rename(tmp_path, path);
   else
      remove(tmp_path);
}

int main(int argc, char **argv)
{
   const char *home = getenv("HOME");
   char config[MAX_PATH_LEN];

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
         fix_config(config);
      }
   }

   /* No EGL on Ubuntu Touch, so SDL2 has to render in software. */
   setenv("SDL_RENDER_DRIVER", "software", 0);

   argv[0] = "retroarch";
   execvp("retroarch", argv);
   perror("retroarch");

   return 1;
}
