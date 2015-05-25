#version 330

in vec2 TexCoord;
in vec3 Normal;

uniform sampler2D Texture;
uniform sampler2D DitherMap;

out vec4 FragColor;

void main() {
    float shade = dot(Normal, vec3(1,0.5,1.0));
    int pal = clamp(int(floor(shade*3.0)),0,2);

    if (pal == 0) {
        shade = 0.2;
    } else if (pal == 2) {
        shade = 1.0;
    } else {
        vec4 dither = texture(DitherMap, vec2(gl_FragCoord.x/800.0,gl_FragCoord.y/600.0));
        if (dither.r > 0.5) {
            shade = 1.0;
        } else {
            shade = 0.2;
        }
    }

    FragColor = texture(Texture, TexCoord) * shade;
    //FragColor = vec4(vec3(texture(DitherMap, TexCoord)),1.0);
    //FragColor = texture(DitherMap, TexCoord);
}
