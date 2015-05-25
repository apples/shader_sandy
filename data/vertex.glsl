#version 330

in vec3 VertexPosition;
in vec2 VertexTexcoord;
in vec3 VertexNormal;

uniform mat4 camProj;
uniform mat4 camView;
uniform mat4 modelPos;

out vec2 TexCoord;
out vec3 Normal;
out vec3 Position;

void main() {
    mat4 MVP = camProj * camView * modelPos;

    mat4 NormalMatrix = transpose(inverse(camView * modelPos));

    TexCoord = VertexTexcoord;
    Normal = (NormalMatrix * vec4(VertexNormal,1.0)).xyz;
    Position = (modelPos * vec4(VertexPosition,1.0)).xyz;

    gl_Position = MVP * vec4(VertexPosition,1.0);
}
