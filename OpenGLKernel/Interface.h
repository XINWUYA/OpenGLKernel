#pragma once
#include "Common.h"

NAMESPACE_BEGIN(gl_kernel)

OPENGL_KERNEL_EXPORT void init();
OPENGL_KERNEL_EXPORT void mainloop();
OPENGL_KERNEL_EXPORT void shutdown();
OPENGL_KERNEL_EXPORT void exit();

NAMESPACE_END(gl_kernel)