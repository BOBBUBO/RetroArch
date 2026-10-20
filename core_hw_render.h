/* Copyright  (C) 2024 - the RetroArch team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (core_hw_render.h).
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

#ifndef __CORE_HW_RENDER_H
#define __CORE_HW_RENDER_H

#include <boolean.h>

#include <retro_common_api.h>

RETRO_BEGIN_DECLS

/* Whether this platform can give a core a hardware rendering context at
 * all. Where it cannot, a core that asks for one has to be refused, and
 * a core that cannot run without one has no business being offered.
 *
 * Ubuntu Touch's compositor implements wl_shm only, so it hands no EGL to
 * Wayland clients: there is no context to give there. */
bool core_hw_render_available(void);

/* Whether the core named by @core_filename is one that needs a hardware
 * rendering context to run. The names come from the environment, in the
 * variable named by CORE_HW_RENDER_NO_HW_ENV, as a comma separated list
 * of core file names: the Ubuntu Touch launcher fills it with the cores
 * that cannot run without one.
 *
 * Only the file name is compared, so a path may be passed. */
bool core_hw_render_required(const char *core_filename);

/* Records that a core asked for a hardware rendering context and was
 * refused one, and whether that happened since the flag was last
 * cleared. */
void core_hw_render_set_refused(bool refused);
bool core_hw_render_get_refused(void);

RETRO_END_DECLS

#endif
