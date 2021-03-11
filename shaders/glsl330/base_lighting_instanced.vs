#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

layout (location = 12) in mat4 instance;

// Input uniform values
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// NOTE: Add here your custom variables

void main()
{
		// instances are supplied transposed
		mat4 instanceT = transpose(instance);

    // Send vertex attributes to fragment shader
    fragPosition = vec3(instanceT * vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    mat3 normalMatrix = transpose(inverse(mat3(instanceT)));
    fragNormal = normalize(normalMatrix * vertexNormal);

    mat4 mvpi = mvp * instanceT;

    // Calculate final vertex position
    gl_Position = mvpi * vec4(vertexPosition, 1.0);
}
