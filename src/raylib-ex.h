#pragma once

#define GRAPHICS_API_OPENGL_33
#define GLSL_VERSION 330
#define PLATFORM_DESKTOP

#include "../raylib/src/config.h"

#undef RL_CULL_DISTANCE_NEAR
#define RL_CULL_DISTANCE_NEAR 1.0

#include "../raylib/src/rlgl.h"
#include "../raylib/src/raylib.h"
#include "../raylib/src/raymath.h"
#include "../raylib/examples/shaders/rlights.h"

extern "C" {
	void* rlWindowHandle();
	void rlDrawMeshInstanced2(Mesh mesh, Material material, int count, Matrix *transforms);
	void rlDrawMaterialMeshes(Material material, int count, Mesh *meshes, Matrix *transforms);
	void rlDrawParticles(Mesh mesh, Material material, int count, Vector4 *particles);
}
