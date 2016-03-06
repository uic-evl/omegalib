uniform float unif_Alpha;
uniform sampler2D unif_Texture;
varying vec2 var_TexCoord;

void main (void)
{
    gl_FragColor = texture2D(unif_Texture, var_TexCoord) * gl_Color;
    gl_FragColor.a *= unif_Alpha;
}