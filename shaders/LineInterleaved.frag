uniform sampler2D leftEyeImage;
uniform sampler2D rightEyeImage;

varying vec2 vTexCoord;

void main() {
	vec3 rgbL = texture2D(leftEyeImage,  vTexCoord).rgb;
	vec3 rgbR = texture2D(rightEyeImage, vTexCoord).rgb;

	if (gl_FragCoord.y/2.0 - float(int(gl_FragCoord.y/2.0)) < 0.5)
		gl_FragColor = vec4(rgbR, 1.0);
	else
		gl_FragColor = vec4(rgbL, 1.0);
}
