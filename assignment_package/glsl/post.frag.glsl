#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

in vec2 fs_UV;
out vec4 out_Col;
uniform sampler2D u_Texture;
uniform int u_Case;

void main()
{
    vec3 col = texture(u_Texture, fs_UV).rgb;

    if (u_Case == 1) {
        col += vec3(0, 0, 1);// in the water
    } else if (u_Case == 2) {
        col += vec3(1, 0, 0);// in the lava
    }
    out_Col = vec4(col, 1);
}
