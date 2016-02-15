uniform sampler2D leftEyeImage;
uniform sampler2D rightEyeImage;

varying vec2 vTexCoord;

void main() {
	vec3 rgbL = texture2D(leftEyeImage,  vTexCoord).rgb;
	vec3 rgbR = texture2D(rightEyeImage, vTexCoord).rgb;

	mat3 anaglyphRCL = mat3( 0.437,  0.449,  0.164,
				-0.062, -0.062, -0.024,
				-0.048, -0.050, -0.017);
	mat3 anaglyphRCR = mat3(-0.011, -0.032, -0.007,
				 0.377,  0.761,  0.009,
				-0.026, -0.093,  1.234);

	vec3 anaglyphLeft  = rgbL * anaglyphRCL;
	vec3 anaglyphRight = rgbR * anaglyphRCR;

	gl_FragColor = vec4(clamp(anaglyphLeft+anaglyphRight, 0.0, 1.0), 1.0);
}
