varying vec2 var_TexCoord;

void main(void)
{
	gl_Position = ftransform();
	gl_FrontColor = gl_Color;
	var_TexCoord = gl_MultiTexCoord0.xy;
}
