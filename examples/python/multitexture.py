from omega import *
from euclid import *
from cyclops import *
from omegaToolkit import *

scene = getSceneManager()
scene.createProgramFromString("multiTexture", 
# Vertex shader
'''
	varying vec2 var_TexCoord;
	void main(void)
	{
		gl_Position = ftransform();
		var_TexCoord = gl_MultiTexCoord0.xy;
	}
''',
# Fragment shader
'''
	varying vec2 var_TexCoord;
	uniform sampler2D texture1;
	uniform sampler2D texture2;
	uniform sampler2D texture3;
	
	uniform float texture1Weight;
	uniform float texture2Weight;
	uniform float texture3Weight;

	void main (void)
	{
		vec4 c1 = texture1Weight * texture2D(texture1, var_TexCoord);
		vec4 c2 = texture2Weight * texture2D(texture2, var_TexCoord);
		vec4 c3 = texture3Weight * texture2D(texture3, var_TexCoord);

		gl_FragColor = c1 + c2 + c3;	
	}
''')

plane = PlaneShape.create(0.8, 0.8)
plane.setPosition(Vector3(1, 2, -3))

# Attach three textures to the plane
material = plane.getMaterial()
material.setProgram("multiTexture")
material.setTexture("cyclops/test/checker.jpg", 0, "texture1")
material.setTexture("cyclops/test/soil0.jpg", 1, "texture2")
material.setTexture("cyclops/test/wall002.jpg", 2, "texture3")

texture1Weight = material.addUniform('texture1Weight', UniformType.Float)
texture2Weight = material.addUniform('texture2Weight', UniformType.Float)
texture3Weight = material.addUniform('texture3Weight', UniformType.Float)
texture1Weight.setFloat(1)
texture2Weight.setFloat(0)
texture3Weight.setFloat(0)

# Add transparency sliders
ui = UiModule.createAndInitialize().getUi()
c = Container.create(ContainerLayout.LayoutVertical, ui)
c.setPosition(Vector2(5, 20))
l1 = Label.create(c)
l1.setText("Texture 1 Opacity")
s1 = Slider.create(c)
s1.setTicks(10)
s1.setUIEventCommand("texture1Weight.setFloat(%value%.0 / 10)")

l2 = Label.create(c)
l2.setText("Texture 2 Opacity")
s2 = Slider.create(c)
s2.setTicks(10)
s2.setUIEventCommand("texture2Weight.setFloat(%value%.0 / 10)")

l3 = Label.create(c)
l3.setText("Texture 3 Opacity")
s3 = Slider.create(c)
s3.setTicks(10)
s3.setUIEventCommand("texture3Weight.setFloat(%value%.0 / 10)")
