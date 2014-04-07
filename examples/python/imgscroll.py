ui = UiModule.createAndInitialize().getUi()

img = loadImage('docs/ovtk.png')

# Scrollable image
l = Label.create(ui)
l.setPosition(Vector2(4, 26))
l.setText("Drag the red box to choose zoom area")
l = Label.create(ui)
l.setPosition(Vector2(5, 42))
l.setText("Drag white box to change the selection size")

overview = Image.create(ui)
overview.setPosition(Vector2(5, 64))
overview.setData(img)
overview.setSize(Vector2(200,200))

sel = Container.create(ContainerLayout.LayoutFree, ui)
sel.setPosition(overview.getPosition())
sel.setDraggable(True)
sel.setStyleValue('fill', '#ff000050')
sel.setStyleValue('border', '2 #ffffff')

scaler = Container.create(ContainerLayout.LayoutFree, sel)
scaler.setDraggable(True)
scaler.setAutosize(False)
scaler.setPosition(Vector2(50,50))
scaler.setSize(Vector2(15,15))
scaler.setStyleValue('fill', 'white')


focus = Image.create(ui)
focus.setPosition(Vector2(280, 5))
focus.setData(img)
focus.setSize(Vector2(450,450))


def onUpdate(frame, time, dt):
    # overview scale x and y
    overviewScaleX = img.getWidth() / overview.getWidth()
    overviewScaleY = img.getHeight() / overview.getHeight()
    
    # lock selection box within bounds of overview widget
    sp = sel.getPosition()
    sps = sel.getSize()
    op = overview.getPosition()
    ops = overview.getSize()
    if(sp.x < op.x):
        sp.x = op.x
        sel.setPosition(sp)
    if(sp.y < op.y):
        sp.y = op.y
        sel.setPosition(sp)
    if(sp.x + sps.x > op.x + ops.x):
        sp.x = op.x + ops.x - sps.x
        sel.setPosition(sp)
    if(sp.y + sps.y > op.y + ops.y):
        sp.y = op.y + ops.y - sps.y
        sel.setPosition(sp)
    
    x = int((sp.x - op.x) * overviewScaleX)
    y = int((sp.y - op.y) * overviewScaleY)
    w = int((sps.x) * overviewScaleX)
    h = int((sps.y) * overviewScaleY)
    
    focus.setSourceRect(x, y, w, h)
    
setUpdateFunction(onUpdate)