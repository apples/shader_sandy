#version 330

in vec2 TexCoord;
in vec3 Normal;
in vec3 Position;

uniform sampler2D Texture;
uniform sampler3D DitherMap;

uniform vec3 LightPos;
uniform float LightRadius;

uniform float ScreenWidth;
uniform float ScreenHeight;

out vec4 FragColor;

void main() {

    /*
    float shade = dot(normalize(Normal), normalize(LightPos));
    float fullBright = 0.8;
    float lowBright = 0.5;
    if (shade > fullBright) {
        shade = 1.0;
    } else if (shade < lowBright) {
        shade = 0.2;
    } else {
        float ditherStrength = 1.0-(shade-lowBright)/(fullBright-lowBright);
        vec4 dither = texture(DitherMap, vec3(gl_FragCoord.x/ScreenWidth,gl_FragCoord.y/ScreenHeight,ditherStrength));
        if (dither.r > 0.5) {
            shade = 1.0;
        } else {
            shade = 0.2;
        }
    }
    */

    float fullBright = 0.8;
    float lowBright = 0.2;
    float shade = clamp((dot(normalize(Normal), normalize(LightPos))-lowBright)/(fullBright-lowBright),0.0,1.0);
    if (shade > 0.0 && shade < 1.0) {
        shade = mix(0.2,1.0,pow(texture(DitherMap, vec3(gl_FragCoord.x/ScreenWidth, gl_FragCoord.y/ScreenHeight, 1.0-shade)).r,2.0));
    } else {
        shade = clamp(shade, 0.2, 1.0);
    }


    /*

    vec3 lpos = LightPos;
    float lrad = LightRadius;
    float dist = length(lpos-Position);
    float shade;

    if (dist < lrad) {
        shade = 1.0;
    } else if (dist > (lrad+0.2)) {
        shade = 0.2;
    } else {
        float ditherStrength = (dist-lrad)/0.2;
        if (texture(DitherMap, vec3(gl_FragCoord.x/640.0,gl_FragCoord.y/480.0,ditherStrength)).r > 0.5) {
            shade = 1.0;
        } else {
            shade = 0.2;
        }
    }
    */

    FragColor = texture(Texture, TexCoord) * shade;
    //FragColor = vec4(vec3(texture(DitherMap, TexCoord)),1.0);
    //FragColor = texture(DitherMap, TexCoord);
}
