
// Jump through a few hoops to make Raylib compile as a local object.
// We have to add a few functions that touch rlgl and raylib internals.

#define GRAPHICS_API_OPENGL_33
#define GLSL_VERSION 330
#define PLATFORM_DESKTOP

#include "raylib/src/config.h"

#undef RL_CULL_DISTANCE_NEAR
#define RL_CULL_DISTANCE_NEAR 1.0

#define EXTERNAL_CONFIG_FLAGS

#include "raylib/src/rlgl.h"
#include "raylib/src/raylib.h"
#include "raylib/src/raymath.h"

#include "raylib/src/core.c"
#undef RLGL_IMPLEMENTATION

#include "raylib/src/shapes.c"
#include "raylib/src/models.c"
#include "raylib/src/textures.c"
#include "raylib/src/text.c"
#include "raylib/src/utils.c"

#define RLIGHTS_IMPLEMENTATION
#include "raylib/examples/shaders/rlights.h"

void* rlWindowHandle() {
	return CORE.Window.handle;
}

// Draw a 3d mesh with material and transform
void rlDrawMeshInstanced2(Mesh mesh, Material material, int count, Matrix *transforms)
{
//	rlDrawMeshInstanced(mesh, material, transforms, count);
#if defined(GRAPHICS_API_OPENGL_33)

    if (!RLGL.ExtSupported.vao) {
        TRACELOG(LOG_ERROR, "VAO: Instanced rendering requires VAO support");
        return;
    }

    // Bind shader program
    glUseProgram(material.shader.id);

    // Upload to shader material.colDiffuse
    if (material.shader.locs[LOC_COLOR_DIFFUSE] != -1)
        glUniform4f(material.shader.locs[LOC_COLOR_DIFFUSE], (float)material.maps[MAP_DIFFUSE].color.r/255.0f,
                                                           (float)material.maps[MAP_DIFFUSE].color.g/255.0f,
                                                           (float)material.maps[MAP_DIFFUSE].color.b/255.0f,
                                                           (float)material.maps[MAP_DIFFUSE].color.a/255.0f);

    // Upload to shader material.colSpecular (if available)
    if (material.shader.locs[LOC_COLOR_SPECULAR] != -1)
        glUniform4f(material.shader.locs[LOC_COLOR_SPECULAR], (float)material.maps[MAP_SPECULAR].color.r/255.0f,
                                                               (float)material.maps[MAP_SPECULAR].color.g/255.0f,
                                                               (float)material.maps[MAP_SPECULAR].color.b/255.0f,
                                                               (float)material.maps[MAP_SPECULAR].color.a/255.0f);

    if (material.shader.locs[LOC_MATRIX_VIEW] != -1)
        SetShaderValueMatrix(material.shader, material.shader.locs[LOC_MATRIX_VIEW], RLGL.State.modelview);

    if (material.shader.locs[LOC_MATRIX_PROJECTION] != -1)
        SetShaderValueMatrix(material.shader, material.shader.locs[LOC_MATRIX_PROJECTION], RLGL.State.projection);

    // Bind active texture maps (if available)
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        if (material.maps[i].texture.id > 0)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            if ((i == MAP_IRRADIANCE) || (i == MAP_PREFILTER) || (i == MAP_CUBEMAP))
                glBindTexture(GL_TEXTURE_CUBE_MAP, material.maps[i].texture.id);
            else glBindTexture(GL_TEXTURE_2D, material.maps[i].texture.id);

            glUniform1i(material.shader.locs[LOC_MAP_DIFFUSE + i], i);
        }
    }

    // Bind vertex array objects (or VBOs)
    glBindVertexArray(mesh.vaoId);

    // At this point the modelview matrix just contains the view matrix (camera)
    // For instanced shaders "mvp" is not premultiplied by any instance transform, only RLGL.State.transform
    glUniformMatrix4fv(material.shader.locs[LOC_MATRIX_MVP], 1, false, MatrixToFloat(
        MatrixMultiply(MatrixMultiply(RLGL.State.transform, RLGL.State.modelview), RLGL.State.projection)
    ));

    float16* instances = RL_MALLOC(count * sizeof(float16));

    for (int i = 0; i < count; i++)
        instances[i] = MatrixToFloatV(transforms[i]);

    // This could alternatively use a static VBO and either glMapBuffer or glBufferSubData.
    // It isn't clear which would be reliably faster in all cases and on all platforms, and
    // anecdotally glMapBuffer seems very slow (syncs) while glBufferSubData seems no faster
    // since we're transferring all the transform matrices anyway.
    unsigned int instancesB;
    glGenBuffers(1, &instancesB);
    glBindBuffer(GL_ARRAY_BUFFER, instancesB);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(float16), instances, GL_DYNAMIC_DRAW);

    // Instances are put in LOC_MATRIX_MODEL attribute location with space for 4x Vector4, eg:
    // layout (location = 12) in mat4 instance;
    unsigned int instanceA = material.shader.locs[LOC_MATRIX_MODEL];

    for (unsigned int i = 0; i < 4; i++)
    {
        glEnableVertexAttribArray(instanceA+i);
        glVertexAttribPointer(instanceA+i, 4, GL_FLOAT, GL_FALSE, sizeof(Matrix), (void*)(i * sizeof(Vector4)));
        glVertexAttribDivisor(instanceA+i, 1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Draw call!
    if (mesh.indices != NULL) {
        // Indexed vertices draw
        glDrawElementsInstanced(GL_TRIANGLES, mesh.triangleCount*3, GL_UNSIGNED_SHORT, 0, count);
    } else {
        glDrawArraysInstanced(GL_TRIANGLES, 0, mesh.vertexCount, count);
    }

    glDeleteBuffers(1, &instancesB);
    RL_FREE(instances);

    // Unbind all binded texture maps
    for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);       // Set shader active texture
        if ((i == MAP_IRRADIANCE) || (i == MAP_PREFILTER) || (i == MAP_CUBEMAP)) glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        else glBindTexture(GL_TEXTURE_2D, 0);   // Unbind current active texture
    }

    // Unind vertex array objects (or VBOs)
    glBindVertexArray(0);

    // Unbind shader program
    glUseProgram(0);

#else
    TRACELOG(LOG_ERROR, "VAO: Instanced rendering requires GRAPHICS_API_OPENGL_33");
#endif
}

// Draw a 3d mesh with material and transform
void rlDrawMaterialMeshes(Material material, int count, Mesh *meshes, Matrix *transforms)
{
#if defined(GRAPHICS_API_OPENGL_33) || defined(GRAPHICS_API_OPENGL_ES2)
	// Bind shader program
	glUseProgram(material.shader.id);

	// Bind active texture maps (if available)
	for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
	{
		if (material.maps[i].texture.id > 0)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			if ((i == MAP_IRRADIANCE) || (i == MAP_PREFILTER) || (i == MAP_CUBEMAP)) glBindTexture(GL_TEXTURE_CUBE_MAP, material.maps[i].texture.id);
			else glBindTexture(GL_TEXTURE_2D, material.maps[i].texture.id);

			glUniform1i(material.shader.locs[LOC_MAP_DIFFUSE + i], i);
		}
	}

	// Upload to shader material.colDiffuse
	if (material.shader.locs[LOC_COLOR_DIFFUSE] != -1)
		glUniform4f(material.shader.locs[LOC_COLOR_DIFFUSE], (float)material.maps[MAP_DIFFUSE].color.r/255.0f,
														   (float)material.maps[MAP_DIFFUSE].color.g/255.0f,
														   (float)material.maps[MAP_DIFFUSE].color.b/255.0f,
														   (float)material.maps[MAP_DIFFUSE].color.a/255.0f);

	// Upload to shader material.colSpecular (if available)
	if (material.shader.locs[LOC_COLOR_SPECULAR] != -1)
		glUniform4f(material.shader.locs[LOC_COLOR_SPECULAR], (float)material.maps[MAP_SPECULAR].color.r/255.0f,
															   (float)material.maps[MAP_SPECULAR].color.g/255.0f,
															   (float)material.maps[MAP_SPECULAR].color.b/255.0f,
															   (float)material.maps[MAP_SPECULAR].color.a/255.0f);

	if (material.shader.locs[LOC_MATRIX_VIEW] != -1)
		glUniformMatrix4fv(material.shader.locs[LOC_MATRIX_VIEW], 1, false, MatrixToFloat(RLGL.State.modelview));

	if (material.shader.locs[LOC_MATRIX_PROJECTION] != -1)
		glUniformMatrix4fv(material.shader.locs[LOC_MATRIX_PROJECTION], 1, false, MatrixToFloat(RLGL.State.projection));

	for (int instance = 0; instance < count; instance++)
	{
		Mesh mesh = meshes[instance];
		Matrix transform = transforms[instance];

		// Bind vertex array objects (or VBOs)
		glBindVertexArray(mesh.vaoId);

		// Calculate and send to shader model matrix (used by PBR shader)
		if (material.shader.locs[LOC_MATRIX_MODEL] != -1)
			glUniformMatrix4fv(material.shader.locs[LOC_MATRIX_MODEL], 1, false, MatrixToFloat(transform));

		// Transform to camera-space coordinates
		// At this point the modelview matrix just contains the view matrix (camera)
		Matrix matModelView = MatrixMultiply(transform, MatrixMultiply(RLGL.State.transform, RLGL.State.modelview));

		// Calculate model-view-projection matrix (MVP)
		Matrix matMVP = MatrixMultiply(matModelView, RLGL.State.projection);

		// Send combined model-view-projection matrix to shader
		glUniformMatrix4fv(material.shader.locs[LOC_MATRIX_MVP], 1, false, MatrixToFloat(matMVP));

		// Draw call!
		if (mesh.indices != NULL) glDrawElements(GL_TRIANGLES, mesh.triangleCount*3, GL_UNSIGNED_SHORT, 0); // Indexed vertices draw
		else glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
	}

	// Unbind all binded texture maps
	for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);       // Set shader active texture
		if ((i == MAP_IRRADIANCE) || (i == MAP_PREFILTER) || (i == MAP_CUBEMAP)) glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		else glBindTexture(GL_TEXTURE_2D, 0);   // Unbind current active texture
	}

	// Unind vertex array objects (or VBOs)
	glBindVertexArray(0);

	// Unbind shader program
	glUseProgram(0);

#endif
}

// Draw a 3d mesh with material and transform
void rlDrawParticles(Mesh mesh, Material material, int count, Vector4 *particles)
{
#if defined(GRAPHICS_API_OPENGL_33)

	if (!RLGL.ExtSupported.vao) {
		//notef("VAO: Instanced rendering requires VAO support");
		return;
	}

	// Bind shader program
	glUseProgram(material.shader.id);

	// Upload to shader material.colDiffuse
	if (material.shader.locs[LOC_COLOR_DIFFUSE] != -1)
		glUniform4f(material.shader.locs[LOC_COLOR_DIFFUSE], (float)material.maps[MAP_DIFFUSE].color.r/255.0f,
														   (float)material.maps[MAP_DIFFUSE].color.g/255.0f,
														   (float)material.maps[MAP_DIFFUSE].color.b/255.0f,
														   (float)material.maps[MAP_DIFFUSE].color.a/255.0f);

	// Upload to shader material.colSpecular (if available)
	if (material.shader.locs[LOC_COLOR_SPECULAR] != -1)
		glUniform4f(material.shader.locs[LOC_COLOR_SPECULAR], (float)material.maps[MAP_SPECULAR].color.r/255.0f,
															   (float)material.maps[MAP_SPECULAR].color.g/255.0f,
															   (float)material.maps[MAP_SPECULAR].color.b/255.0f,
															   (float)material.maps[MAP_SPECULAR].color.a/255.0f);

	if (material.shader.locs[LOC_MATRIX_VIEW] != -1)
		SetShaderValueMatrix(material.shader, material.shader.locs[LOC_MATRIX_VIEW], RLGL.State.modelview);

	if (material.shader.locs[LOC_MATRIX_PROJECTION] != -1)
		SetShaderValueMatrix(material.shader, material.shader.locs[LOC_MATRIX_PROJECTION], RLGL.State.projection);

	// Bind active texture maps (if available)
	for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
	{
		if (material.maps[i].texture.id > 0)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			if ((i == MAP_IRRADIANCE) || (i == MAP_PREFILTER) || (i == MAP_CUBEMAP))
				glBindTexture(GL_TEXTURE_CUBE_MAP, material.maps[i].texture.id);
			else glBindTexture(GL_TEXTURE_2D, material.maps[i].texture.id);

			glUniform1i(material.shader.locs[LOC_MAP_DIFFUSE + i], i);
		}
	}

	// Bind vertex array objects (or VBOs)
	glBindVertexArray(mesh.vaoId);

	// At this point the modelview matrix just contains the view matrix (camera)
	// For instanced shaders "mvp" is not premultiplied by any instance transform, only RLGL.State.transform
	glUniformMatrix4fv(material.shader.locs[LOC_MATRIX_MVP], 1, false, MatrixToFloat(
		MatrixMultiply(MatrixMultiply(RLGL.State.transform, RLGL.State.modelview), RLGL.State.projection)
	));

	// This could alternatively use a static VBO and either glMapBuffer or glBufferSubData.
	// It isn't clear which would be reliably faster in all cases and on all platforms, and
	// anecdotally glMapBuffer seems very slow (syncs) while glBufferSubData seems no faster
	// since we're transferring all the transform matrices anyway.
	unsigned int instancesB;
	glGenBuffers(1, &instancesB);
	glBindBuffer(GL_ARRAY_BUFFER, instancesB);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(Vector4), particles, GL_DYNAMIC_DRAW);

	// Instances are put in LOC_MATRIX_MODEL attribute location
	unsigned int instanceA = material.shader.locs[LOC_MATRIX_MODEL];
	glEnableVertexAttribArray(instanceA);
	glVertexAttribPointer(instanceA, 4, GL_FLOAT, GL_FALSE, sizeof(Vector4), (void*)(sizeof(Vector4)));
	glVertexAttribDivisor(instanceA, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Draw call!
	if (mesh.indices != NULL) {
		// Indexed vertices draw
		glDrawElementsInstanced(GL_TRIANGLES, mesh.triangleCount*3, GL_UNSIGNED_SHORT, 0, count);
	} else {
		glDrawArraysInstanced(GL_TRIANGLES, 0, mesh.vertexCount, count);
	}

	glDeleteBuffers(1, &instancesB);

	// Unbind all binded texture maps
	for (int i = 0; i < MAX_MATERIAL_MAPS; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);       // Set shader active texture
		if ((i == MAP_IRRADIANCE) || (i == MAP_PREFILTER) || (i == MAP_CUBEMAP)) glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		else glBindTexture(GL_TEXTURE_2D, 0);   // Unbind current active texture
	}

	// Unind vertex array objects (or VBOs)
	glBindVertexArray(0);

	// Unbind shader program
	glUseProgram(0);

#endif
}
