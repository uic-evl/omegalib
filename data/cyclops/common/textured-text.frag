// Applies a texture to the emissive channel instead of the diffuse channel.
@surfaceShader 

// The diffuse texture
uniform sampler2D unif_DiffuseMap;
varying vec2 var_TexCoord;

uniform float unif_Shininess;
uniform float unif_Gloss;

varying vec3 var_Normal;

///////////////////////////////////////////////////////////////////////////////////////////////////
SurfaceData getSurfaceData(void)
{
	SurfaceData sd;
    	sd.albedo = vec4(0, 0, 0, 1);
	sd.emissive = gl_Color * texture2D(unif_DiffuseMap, var_TexCoord).a; 
	sd.shininess = unif_Shininess;
	sd.gloss = unif_Gloss;
	sd.normal = var_Normal;
	
	return sd;
}
