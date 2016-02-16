attribute vec3 aVertexPosition;
attribute vec2 aVertexTexCoord;

varying vec2 vTexCoord;

void main() {
	vTexCoord = aVertexTexCoord;
	gl_Position = vec4(aVertexPosition, 1.0);
}
