#version 330

in vec2 TexCoord;
in vec3 Normal;
in vec3 Position;

uniform sampler2D Texture;
uniform sampler2D DitherMap[5];

uniform vec3 LightPos;
uniform float LightRadius;

out vec4 FragColor;

void main() {
    /*
    float shade = dot(Normal, vec3(0.5,0.1,1.0));
    int pal = clamp(int(shade*7.0),0,6);
    if (pal == 0 || pal == 1) {
        shade = 0.2;
    } else if (pal == 6 || pal == 5) {
        shade = 1.0;
    } else {
        vec4 dither = texture(DitherMap[pal-1], vec2(gl_FragCoord.x/640.0,gl_FragCoord.y/480.0));
        if (dither.r > 0.5) {
            shade = 1.0;
        } else {
            shade = 0.2;
        }
    }
    */

    vec3 lpos = LightPos;
    float lrad = LightRadius;
    float dist = length(lpos-Position);
    float shade;

    if (dist < lrad) {
        shade = 1.0;
    } else if (dist > (lrad+0.2)) {
        shade = 0.2;
    } else {
        int pal = clamp(int(4.0*(dist-lrad)/0.2),0,3);
        if (texture(DitherMap[3-pal], vec2(gl_FragCoord.x/640.0,gl_FragCoord.y/480.0)).r > 0.5) {
            shade = 1.0;
        } else {
            shade = 0.2;
        }
    }

    FragColor = texture(Texture, TexCoord) * shade;
    //FragColor = vec4(vec3(texture(DitherMap, TexCoord)),1.0);
    //FragColor = texture(DitherMap, TexCoord);
}
