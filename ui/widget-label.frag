uniform float unif_Alpha;
uniform sampler2D unif_Texture;
varying vec2 var_TexCoord;

void main (void)
{
	// For text, we use the texture just as an alpha mask.
	float a = texture2D(unif_Texture, var_TexCoord).a;
    gl_FragColor = gl_Color;
    gl_FragColor.a *= (a * unif_Alpha);
}