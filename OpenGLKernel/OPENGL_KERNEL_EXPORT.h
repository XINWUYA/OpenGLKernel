#pragma once
#ifndef OPENGL_KERNEL_EXPORTS
#define OPENGL_KERNEL_EXPORT __declspec(dllimport)
#else
#define OPENGL_KERNEL_EXPORT __declspec(dllexport)
#endif