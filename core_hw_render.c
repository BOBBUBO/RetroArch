/* Copyright  (C) 2024 - the RetroArch team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (core_hw_render.c).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * ---------------------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>

#include <core_hw_render.h>

/* The cores that cannot run without a hardware rendering context, as a
 * comma separated list of core file names. The platform that cannot
 * give one at all fills it in: on Ubuntu Touch the launcher does, as
 * its compositor hands no EGL to Wayland clients. */
#define CORE_HW_RENDER_NO_HW_ENV "RETROARCH_NO_HW_CORES"

static bool core_hw_render_refused_flag = false;

bool core_hw_render_available(void)
{
#if defined(UTOUCH)
   /* Ubuntu Touch's compositor implements wl_shm only: it hands no EGL
    * to Wayland clients, so there is no context a core could be given. */
   return false;
#else
   return true;
#endif
}

bool core_hw_render_required(const char *core_filename)
{
   const char *list;
   const char *name;
   size_t name_len;

   if (!core_filename || !*core_filename)
      return false;

   list = getenv(CORE_HW_RENDER_NO_HW_ENV);
   if (!list || !*list)
      return false;

   /* Only the file name is compared: the list holds file names, and
    * the callers have a path or a listing entry. */
   name = strrchr(core_filename, '/');
   name = name ? name + 1 : core_filename;
   name_len = strlen(name);

   while (*list)
   {
      const char *end = strchr(list, ',');
      size_t item_len = end ? (size_t)(end - list) : strlen(list);

      if (item_len == name_len && !strncmp(list, name, name_len))
         return true;

      if (!end)
         break;

      list = end + 1;
   }

   return false;
}

void core_hw_render_set_refused(bool refused)
{
   core_hw_render_refused_flag = refused;
}

bool core_hw_render_get_refused(void)
{
   return core_hw_render_refused_flag;
}
