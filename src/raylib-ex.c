
// Jump through a few hoops to make Raylib compile as a local object.
// We have to add a few functions that touch rlgl and raylib internals.

#define GRAPHICS_API_OPENGL_33
#define GLSL_VERSION 330
#define PLATFORM_DESKTOP
#define RL_MALLOC malloc
#define RL_FREE free

#include "../raylib/src/config.h"

#undef RL_CULL_DISTANCE_NEAR
#define RL_CULL_DISTANCE_NEAR 1.0

#define EXTERNAL_CONFIG_FLAGS

#include "../raylib/src/rlgl.h"
#include "../raylib/src/raylib.h"
#include "../raylib/src/raymath.h"

#include "../raylib/src/core.c"
#undef RLGL_IMPLEMENTATION

#include "../raylib/src/shapes.c"
#include "../raylib/src/models.c"
#include "../raylib/src/textures.c"
#include "../raylib/src/text.c"
#include "../raylib/src/utils.c"

#define RLIGHTS_IMPLEMENTATION
#include "../raylib/examples/shaders/rlights.h"

float clamp(float f, float lo, float hi) {
	if (f < lo) f = lo;
	if (f > hi) f = hi;
	return f;
}

Color GetColorSRGB(unsigned int hexValue)
{
    Color color;

    unsigned char ru = ((unsigned int)hexValue >> 24) & 0xFF;
    unsigned char gu = ((unsigned int)hexValue >> 16) & 0xFF;
    unsigned char bu = ((unsigned int)hexValue >>  8) & 0xFF;
    unsigned char au = ((unsigned int)hexValue >>  0) & 0xFF;

    float rf = (float)ru / 255.0f;
    float gf = (float)gu / 255.0f;
    float bf = (float)bu / 255.0f;

    color.r = clamp(pow(rf, 2.2) * 255.0f, 0, 255);
    color.g = clamp(pow(gf, 2.2) * 255.0f, 0, 255);
    color.b = clamp(pow(bf, 2.2) * 255.0f, 0, 255);
    color.a = au;

    return color;
}

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

//    float16* instances = RL_MALLOC(count * sizeof(float16));
//
//    for (int i = 0; i < count; i++)
//        instances[i] = MatrixToFloatV(transforms[i]);

    assert(sizeof(float16) == sizeof(Matrix));

    // This could alternatively use a static VBO and either glMapBuffer or glBufferSubData.
    // It isn't clear which would be reliably faster in all cases and on all platforms, and
    // anecdotally glMapBuffer seems very slow (syncs) while glBufferSubData seems no faster
    // since we're transferring all the transform matrices anyway.
    unsigned int instancesB;
    glGenBuffers(1, &instancesB);
    glBindBuffer(GL_ARRAY_BUFFER, instancesB);
    glBufferData(GL_ARRAY_BUFFER, count * sizeof(Matrix), transforms, GL_DYNAMIC_DRAW);

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
    //RL_FREE(instances);

    // Unbind all bound texture maps
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

// Generate a mesh from heightmap
Mesh GenMeshHeightmap2(int edge, float *heightmap)
{
    Mesh mesh = { 0 };
    memset(&mesh, 0, sizeof(Mesh));

    int mapX = edge;
    int mapZ = edge;

    // NOTE: One vertex per pixel
    mesh.triangleCount = (mapX-1)*(mapZ-1)*2;    // One quad every four pixels

    mesh.vertexCount = mesh.triangleCount*3;

    mesh.vertices = (float *)calloc(mesh.vertexCount*3, sizeof(float));
    mesh.normals = (float *)calloc(mesh.vertexCount*3, sizeof(float));
    mesh.texcoords = (float *)calloc(mesh.vertexCount*2, sizeof(float));
    mesh.colors = NULL;

    int vCounter = 0;       // Used to count vertices float by float
    int tcCounter = 0;      // Used to count texcoords float by float
    int nCounter = 0;       // Used to count normals float by float

    int trisCounter = 0;

    Vector3 vA;
    Vector3 vB;
    Vector3 vC;
    Vector3 vN;

    for (int z = 0, zo = 0; z < mapZ-1; z++, zo = z*mapX)
    {
        for (int x = 0; x < mapX-1; x++)
        {
            // Fill vertices array with data
            //----------------------------------------------------------

            // one triangle - 3 vertex
            mesh.vertices[vCounter] = (float)x;
            mesh.vertices[vCounter + 1] = (float)heightmap[x + zo];
            mesh.vertices[vCounter + 2] = (float)z;

            mesh.vertices[vCounter + 3] = (float)x;
            mesh.vertices[vCounter + 4] = (float)heightmap[x + zo + mapX];
            mesh.vertices[vCounter + 5] = (float)(z + 1);

            mesh.vertices[vCounter + 6] = (float)(x + 1);
            mesh.vertices[vCounter + 7] = (float)heightmap[(x + 1) + zo];
            mesh.vertices[vCounter + 8] = (float)z;

            // another triangle - 3 vertex
            mesh.vertices[vCounter + 9] = mesh.vertices[vCounter + 6];
            mesh.vertices[vCounter + 10] = mesh.vertices[vCounter + 7];
            mesh.vertices[vCounter + 11] = mesh.vertices[vCounter + 8];

            mesh.vertices[vCounter + 12] = mesh.vertices[vCounter + 3];
            mesh.vertices[vCounter + 13] = mesh.vertices[vCounter + 4];
            mesh.vertices[vCounter + 14] = mesh.vertices[vCounter + 5];

            mesh.vertices[vCounter + 15] = (float)(x + 1);
            mesh.vertices[vCounter + 16] = (float)heightmap[(x + 1) + zo + mapX];
            mesh.vertices[vCounter + 17] = (float)(z + 1);
            vCounter += 18;     // 6 vertex, 18 floats

            // Fill texcoords array with data
            //--------------------------------------------------------------
            mesh.texcoords[tcCounter] = (float)x/(mapX - 1);
            mesh.texcoords[tcCounter + 1] = (float)z/(mapZ - 1);

            mesh.texcoords[tcCounter + 2] = (float)x/(mapX - 1);
            mesh.texcoords[tcCounter + 3] = (float)(z + 1)/(mapZ - 1);

            mesh.texcoords[tcCounter + 4] = (float)(x + 1)/(mapX - 1);
            mesh.texcoords[tcCounter + 5] = (float)z/(mapZ - 1);

            mesh.texcoords[tcCounter + 6] = mesh.texcoords[tcCounter + 4];
            mesh.texcoords[tcCounter + 7] = mesh.texcoords[tcCounter + 5];

            mesh.texcoords[tcCounter + 8] = mesh.texcoords[tcCounter + 2];
            mesh.texcoords[tcCounter + 9] = mesh.texcoords[tcCounter + 3];

            mesh.texcoords[tcCounter + 10] = (float)(x + 1)/(mapX - 1);
            mesh.texcoords[tcCounter + 11] = (float)(z + 1)/(mapZ - 1);
            tcCounter += 12;    // 6 texcoords, 12 floats

            // Fill normals array with data
            //--------------------------------------------------------------
            for (int i = 0; i < 18; i += 9)
            {
                vA.x = mesh.vertices[nCounter + i];
                vA.y = mesh.vertices[nCounter + i + 1];
                vA.z = mesh.vertices[nCounter + i + 2];

                vB.x = mesh.vertices[nCounter + i + 3];
                vB.y = mesh.vertices[nCounter + i + 4];
                vB.z = mesh.vertices[nCounter + i + 5];

                vC.x = mesh.vertices[nCounter + i + 6];
                vC.y = mesh.vertices[nCounter + i + 7];
                vC.z = mesh.vertices[nCounter + i + 8];

                vN = Vector3Normalize(Vector3CrossProduct(Vector3Subtract(vB, vA), Vector3Subtract(vC, vA)));

                mesh.normals[nCounter + i] = vN.x;
                mesh.normals[nCounter + i + 1] = vN.y;
                mesh.normals[nCounter + i + 2] = vN.z;

                mesh.normals[nCounter + i + 3] = vN.x;
                mesh.normals[nCounter + i + 4] = vN.y;
                mesh.normals[nCounter + i + 5] = vN.z;

                mesh.normals[nCounter + i + 6] = vN.x;
                mesh.normals[nCounter + i + 7] = vN.y;
                mesh.normals[nCounter + i + 8] = vN.z;
            }

            nCounter += 18;     // 6 vertex, 18 floats
            trisCounter += 2;
        }
    }

    return mesh;
}

// Unload mesh from memory
void UnloadMesh2(Mesh mesh)
{
    RL_FREE(mesh.vertices);
    RL_FREE(mesh.texcoords);
    RL_FREE(mesh.normals);
    RL_FREE(mesh.colors);
    RL_FREE(mesh.tangents);
    RL_FREE(mesh.texcoords2);
    RL_FREE(mesh.indices);

    RL_FREE(mesh.animVertices);
    RL_FREE(mesh.animNormals);
    RL_FREE(mesh.boneWeights);
    RL_FREE(mesh.boneIds);
}