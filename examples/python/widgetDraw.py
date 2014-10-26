from omegaToolkit import *

uim = UiModule.createAndInitialize()
c = Widget.create(uim.getUi())
c.setDraggable(True)
c.setSize(Vector2(200,200))

firstTime = True
basicShader = 0
textShader = 0

def onDraw(w, camera, painter):
    # Initialize the shaders on first run
    global firstTime, basicShader, textShader
    if(firstTime):
        firstTime = False
        basicShader = painter.getOrCreateProgram('base', 'ui/widget-base.vert', 'ui/widget-base.frag')    
        textShader = painter.getOrCreateProgram('text', 'ui/widget-label.vert', 'ui/widget-label.frag')    
    
    # Draw a rect
    painter.program(basicShader)
    a = painter.findUniform(basicShader, 'unif_Alpha')
    # NOTE: uniformFloat needs to be called after program()
    painter.uniformFloat(a, 1.0)
    painter.drawRect(Vector2(0,0), Vector2(200,200), Color(0, 0.5, 0.5, 1))
    
    # Draw Text
    font = painter.getDefaultFont()
    painter.program(textShader)
    a = painter.findUniform(textShader, 'unif_Alpha')
    painter.uniformFloat(a, 1.0)
    painter.drawText("(Drag me): " + str(w.getPosition()), font, Vector2(5, 5), TextAlign.HALeft | TextAlign.VATop, Color(1, 1, 1, 1))
    
c.setPostDrawCallback(onDraw)    
