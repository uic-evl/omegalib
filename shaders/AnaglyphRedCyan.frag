uniform sampler2D leftEyeImage;
uniform sampler2D rightEyeImage;

varying vec2 vTexCoord;

#define M_PI 3.1415926535897932384626433832795


vec3 processLeftImage(vec3 rgb, vec3 gb, mat3 c, mat3 a);
vec3 processRightImage(vec3 rgb, mat3 c, mat3 a, mat3 aInv);
vec3 rgb2lab(vec3 rgb, mat3 c);
vec3 lab2rgb(vec3 rgb, mat3 a, mat3 aInv);
vec3 gammaForward(vec3 rgb);
float gammaCorrection(float val);
vec3 gammaBackward(vec3 rgb);
float gammaCorrectionInv(float val);
float f(float t);
float fInv(float t);
vec2 getHueSaturation(float a, float b);
float getL(float l, float h, float s);
float tuneLightness(float l, float hueAbsDiff, float t, float s);
vec2 getAB(float h, float s);
vec2 projectCircle(float s);
vec2 projectLine(float s);

void main() {
	vec3 rgbL = texture2D(leftEyeImage,  vTexCoord).rgb;
	vec3 rgbR = texture2D(rightEyeImage, vTexCoord).rgb;

/*
	float g = rgbL.g + 0.45 * max(0.0, rgbL.r - rgbL.g);
	float b = rgbL.b + 0.25 * max(0.0, rgbL.r - rgbL.b);
	float r = g * 0.749 + b * 0.251;
	g = rgbR.g + 0.45 * max(0.0, rgbR.r - rgbR.g);
	b = rgbR.b + 0.25 * max(0.0, rgbR.r - rgbR.b);

	gl_FragColor = vec4(clamp(r, 0.0, 1.0), clamp(g, 0.0, 1.0), clamp(b, 0.0, 1.0), 1.0);
*/

	// Anaglyph method by Songnan Li, Lin Ma, and King Ngi Ngan
	// Gamma correction
	vec3 rgbL2 = gammaForward(rgbL);
	vec3 rgbR2 = gammaForward(rgbR);

	// RGB-CIELAB conversion matrix
	mat3 c = mat3(  0.4243, 0.2492, 0.0265,  // LCD
					0.3105, 0.6419, 0.1225,
					0.1657, 0.1089, 0.8614);
	mat3 Al = mat3( 0.1840, 0.0876, 0.0005,  // Red lens
					0.0179, 0.0118, 0.0012,
					0.0048, 0.0018, 0.0159);
	mat3 Ar = mat3( 0.0153, 0.0176, 0.0201,  // Cyan lens
					0.1092, 0.3088, 0.1016,
					0.1171, 0.0777, 0.6546);
	mat3 ArInv = mat3(142.09766, -7.28549, -3.23244,
					 - 43.58842,  5.60477,  0.46850,
					 - 20.24567,  0.63801,  2.05028);

	// Anaglyph processing
	vec3 gb = processRightImage(rgbR2, c, Ar, ArInv);
	vec3 r = processLeftImage(rgbL2, gb, c, Al);
	vec3 colorOut = gammaBackward(r + gb);

	// Set final pixel color
	gl_FragColor = vec4(colorOut, 1.0);
}

vec3 processLeftImage(vec3 rgb, vec3 gb, mat3 c, mat3 a) {
	vec3 lab = rgb2lab(rgb, c);
	vec3 xyzW = a * vec3(1.0, 1.0, 1.0);
	float Y = fInv((16.0+lab.x) / 116.0) * xyzW.y;

	vec3 rgbOut = vec3(max(Y - 0.0118*gb.g - 0.0018*gb.b,0.0) / 0.0876, 0.0, 0.0);

	return rgbOut;
}

vec3 processRightImage(vec3 rgb, mat3 c, mat3 a, mat3 aInv) {
	vec3 lab = rgb2lab(rgb, c);
	vec2 hs = getHueSaturation(lab.y, lab.z);
	float l2 = getL(lab.x, hs.x, hs.y);
	vec2 ab = getAB(hs.x, hs.y);

	vec3 rgbOut = clamp(lab2rgb(vec3(l2, ab.x, ab.y), a, aInv), 0.0, 1.0);
	rgbOut.r = 0.0;

	return rgbOut;
}

vec3 rgb2lab(vec3 rgb, mat3 c) {
	vec3 white = vec3(1.0, 1.0, 1.0);
	vec3 xyz = c * rgb;
	vec3 xyzW = c * white;

	return vec3(116.0 * f(xyz.y / xyzW.y) - 16.0, 500.0 * (f(xyz.x / xyzW.x) - f(xyz.y / xyzW.y)), 200.0 * (f(xyz.y / xyzW.y) - f(xyz.z / xyzW.z)));
}

vec3 lab2rgb(vec3 lab, mat3 a, mat3 aInv) {
	vec3 white = vec3(1.0, 1.0, 1.0);
	vec3 xyzW = a * white;
	vec3 xyz = vec3(xyzW.x * fInv((lab.x + 16.0) / 116.0 + lab.y / 500.0), xyzW.y * fInv((lab.x + 16.0) / 116.0), xyzW.z * fInv((lab.x + 16.0) / 116.0 - lab.z / 200.0));

	return aInv * xyz;
}

vec3 gammaForward(vec3 rgb) {
	return vec3(gammaCorrection(rgb.r), gammaCorrection(rgb.g), gammaCorrection(rgb.b));
}

float gammaCorrection(float val) {
	if (val <= 0.04045)
		return val / 12.92;
	return pow((val + 0.055) / 1.055, 2.4);
}

vec3 gammaBackward(vec3 rgb) {
	return vec3(gammaCorrectionInv(rgb.r), gammaCorrectionInv(rgb.g), gammaCorrectionInv(rgb.b));
}

float gammaCorrectionInv(float val) {
	if (val <= 0.0031308)
		return 12.92 * val;
	return 1.055 * pow(val, 0.41666) - 0.055;
}

float f(float t) {
	if (t > 0.008856) {
		return pow(t, 1.0/3.0);
	}
	return 7.787 * t + 0.1379;
}

float fInv(float t) {
	if (t > 0.2069) {
		return t*t*t;
	}
	return 0.1284 * (t - 0.1379);
}

vec2 getHueSaturation(float a, float b) {
	return vec2((180.0 / M_PI) * atan(b / (a + 0.0000000001)) + 180.0 * (a < 0.0 ? 1.0 : 0.0), sqrt(a*a + b*b));
}

float getL(float l, float h, float s) {
	float hRed = 41.6;
	float t = 15.0;
	float hueAbsDiff = abs(h - hRed);

	if (hueAbsDiff <= t) {
		return tuneLightness(l, hueAbsDiff, t, s);
	}
	return l;
}

float tuneLightness(float l, float hueAbsDiff, float t, float s) {
	float sL = 40.0;                    
    float sH = 50.0;
    float pMax = 0.4;
    float pSr;

    if (s > sH) {
    	pSr = pMax;
    }
    else if (s < sL) {
    	pSr = 0.0;
    }
    else {
    	pSr = pMax * ((s - sL) / (sH - sL));
    }

    return l * (1.0 - pSr * (t - hueAbsDiff) / t);
}

vec2 getAB(float h, float s) {
	float hRed = 41.6;
	float hCyan = 221.6;
	float t = 15.0;

	float hueAbsDiff = min(abs(h-hRed), abs(h - hCyan));
	if (hueAbsDiff <= t) {
		s = s * (hueAbsDiff / t);
	}

	if (h > hRed && h < hCyan) {
		return projectCircle(s);
	}
	return projectLine(s);
}

vec2 projectCircle(float s) {
	float cA = 125.0;
	float cB = 172.0;

	float a = (4.0 * cA * s*s - 4.0 * cB * s * sqrt(4.0 * cA*cA + 4.0 * cB*cB - s*s)) / (8.0 * cA*cA + 8.0 * cB*cB);
	float b = sqrt(s*s - a*a);

	return vec2(a, b);
}

vec2 projectLine(float s) {
	float k = -0.7273;

    float a = sqrt(s*s / (1.0 + k*k));
    float b = k * a;

    return vec2(a, b);
}
