uniform sampler2D leftEyeImage;
uniform sampler2D rightEyeImage;

varying vec2 vTexCoord;

void main() {
	vec3 rgbL = texture2D(leftEyeImage,  vTexCoord).rgb;
	vec3 rgbR = texture2D(rightEyeImage, vTexCoord).rgb;

	mat3 anaglyphGML = mat3(-0.062, -0.158, -0.039,
				 0.284,  0.668,  0.143,
				-0.015, -0.027,  0.021);
	mat3 anaglyphGMR = mat3( 0.529,  0.705,  0.024,
				-0.016, -0.015,  0.065,
				 0.009,  0.075,  0.937);

	vec3 anaglyphLeft  = rgbL * anaglyphGML;
	vec3 anaglyphRight = rgbR * anaglyphGMR;

	gl_FragColor = vec4(clamp(anaglyphLeft+anaglyphRight, 0.0, 1.0), 1.0);
}
