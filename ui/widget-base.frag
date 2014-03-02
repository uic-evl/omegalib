uniform float unif_Alpha;

void main (void)
{
    gl_FragColor = gl_Color;
    gl_FragColor.a *= unif_Alpha;
}