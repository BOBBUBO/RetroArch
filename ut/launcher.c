#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PATH_LEN 4096

/* The cores that cannot run without a hardware rendering context, and
 * so cannot run here at all: the compositor hands no EGL to Wayland
 * clients, so there is no context to give them. The core downloader
 * leaves them out, and a content that names one is refused. */
#define NO_HW_RENDER_CORES \
   "parallel_n64_libretro.so," \
   "snes9x_libretro.so," \
   "snes9x2010_libretro.so," \
   "mgba_libretro.so," \
   "mednafen_psx_hw_libretro.so," \
   "swanstation_libretro.so," \
   "ppsspp_libretro.so," \
   "dolphin_libretro.so," \
   "flycast_libretro.so," \
   "citra_libretro.so," \
   "citra2018_libretro.so," \
   "cemu_libretro.so," \
   "dosbox_pure_libretro.so," \
   "melonds_libretro.so," \
   "rpcs3_libretro.so," \
   "pcsx2_libretro.so," \
   "kronos_libretro.so"

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

static void copy_str(char *dst, size_t size, const char *src)
{
   size_t len = strlen(src);

   if (len >= size)
      len = size - 1;

   memcpy(dst, src, len);
   dst[len] = '\0';
}

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

/* The value of a setting in the configuration file, or NULL when it is
 * not there. The caller owns the returned string. */
static char *read_setting(FILE *file, const char *key)
{
   char line[1024];
   size_t key_len = strlen(key);

   rewind(file);

   while (fgets(line, sizeof(line), file))
   {
      char *value;

      if (strncmp(line, key, key_len) != 0)
         continue;
      if (strncmp(line + key_len, " = \"", 4) != 0)
         continue;

      value = line + key_len + 4;
      value = strchr(value, '"');
      if (!value)
         continue;

      *value = '\0';

      return strdup(line + key_len + 4);
   }

   return NULL;
}

/* The directory the package is installed in: the launcher is in it, and
 * the overlay it ships is read from there. */
static int find_package_dir(char *out, size_t size)
{
   const char *path = getenv("PATH");
   char entry[MAX_PATH_LEN];
   char candidate[MAX_PATH_LEN];

   if (!path || !*path)
      return 0;

   while (*path)
   {
      const char *end = strchr(path, ':');
      size_t len = end ? (size_t)(end - path) : strlen(path);

      if (len && len < sizeof(entry))
      {
         memcpy(entry, path, len);
         entry[len] = '\0';

         join_path(candidate, sizeof(candidate), entry, "retroarch-launcher");

         if (candidate[0] && access(candidate, R_OK) == 0)
         {
            copy_str(out, size, entry);
            return 1;
         }
      }

      if (!end)
         break;

      path = end + 1;
   }

   out[0] = '\0';

   return 0;
}

/* Rewrite the settings a previous install left behind that cannot work
 * here, so the app does not keep failing on its old configuration. */
static void fix_config(const char *config_dir, const char *package_dir)
{
   char path[MAX_PATH_LEN];
   char tmp_path[MAX_PATH_LEN];
   FILE *in;
   FILE *out;
   char line[1024];
   int changed = 0;
   int set_overlay = 0;

   join_path(path, sizeof(path), config_dir, "retroarch/retroarch.cfg");
   join_path(tmp_path, sizeof(tmp_path), config_dir,
         "retroarch/retroarch.cfg.tmp");

   if (!path[0] || !tmp_path[0])
      return;

   in = fopen(path, "r");
   if (!in)
      return;

   /* An overlay that is not set is the one the package ships: the
    * touch controls are the only way to play here. */
   if (package_dir && *package_dir)
   {
      char *overlay = read_setting(in, "input_overlay");

      if (overlay && !*overlay)
         set_overlay = 1;

      free(overlay);
   }

   out = fopen(tmp_path, "w");
   if (!out)
   {
      fclose(in);
      return;
   }

   rewind(in);

   while (fgets(line, sizeof(line), in))
   {
      const struct setting_fix *fix = find_fix(line);

      if (fix)
      {
         fprintf(out, "%s = \"%s\"\n", fix->key, fix->to);
         changed = 1;
      }
      else if (set_overlay && strncmp(line, "input_overlay = \"", 17) == 0)
      {
         char overlay_path[MAX_PATH_LEN];

         join_path(overlay_path, sizeof(overlay_path), package_dir,
               "overlays/snes.cfg");

         if (overlay_path[0])
         {
            fprintf(out, "input_overlay = \"%s\"\n", overlay_path);
            changed = 1;
         }
         else
            fputs(line, out);
      }
      else if (set_overlay
            && strncmp(line, "input_overlay_enable = \"", 24) == 0)
      {
         fprintf(out, "input_overlay_enable = \"true\"\n");
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
   char package_dir[MAX_PATH_LEN];

   (void)argc;

   package_dir[0] = '\0';
   find_package_dir(package_dir, sizeof(package_dir));

   /* Click profiles cannot execute /usr/bin/env; set the config dir here. */
   if (home && *home)
   {
      static const char suffix[] = "/.config/com.libretro.retroarch";
      static const char cookie[] = "/.config/pulse/cookie";
      char pulse[MAX_PATH_LEN];
      size_t len = strlen(home);

      if (len + sizeof(suffix) <= sizeof(config))
      {
         memcpy(config, home, len);
         memcpy(config + len, suffix, sizeof(suffix));
         setenv("XDG_CONFIG_HOME", config, 1);
         fix_config(config, package_dir);
      }

      /* PulseAudio keeps the client cookie in the user's own
       * configuration directory, which the directory above moves out of
       * the way: point the client at it where it really is. */
      if (len + sizeof(cookie) <= sizeof(pulse))
      {
         memcpy(pulse, home, len);
         memcpy(pulse + len, cookie, sizeof(cookie));
         setenv("PULSE_COOKIE", pulse, 0);
      }
   }

   /* No EGL on Ubuntu Touch, so SDL2 has to render in software. */
   setenv("SDL_RENDER_DRIVER", "software", 0);

   /* The cores that need a hardware rendering context cannot run here. */
   setenv("RETROARCH_NO_HW_CORES", NO_HW_RENDER_CORES, 0);

   argv[0] = "retroarch";
   execvp("retroarch", argv);
   perror("retroarch");

   return 1;
}
