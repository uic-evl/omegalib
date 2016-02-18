uniform sampler2D leftEyeImage;
uniform sampler2D rightEyeImage;

varying vec2 vTexCoord;

void main() {
	vec3 rgbL = texture2D(leftEyeImage,  vTexCoord).rgb;
	vec3 rgbR = texture2D(rightEyeImage, vTexCoord).rgb;

	float g = rgbL.g + 0.45 * max(0.0, rgbL.r - rgbL.g);
	float b = rgbL.b + 0.25 * max(0.0, rgbL.r - rgbL.b);
	float r = g * 0.749 + b * 0.251;
	g = rgbR.g + 0.45 * max(0.0, rgbR.r - rgbR.g);
	b = rgbL.b + 0.25 * max(0.0, rgbL.r - rgbL.b);

	gl_FragColor = vec4(clamp(r, 0.0, 1.0), clamp(g, 0.0, 1.0), clamp(b, 0.0, 1.0), 1.0);
}
