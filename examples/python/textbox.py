ui = UiModule.createAndInitialize().getUi()

c = Container.create(ContainerLayout.LayoutVertical, ui)
c.setSize(Vector2(300, 200))
c.setPosition(Vector2(5, 32))
c.setFillEnabled(True)
c.setFillColor(Color('#202020'))

l1 = Label.create(c)
l1.setText("First Name:")
t1 = TextBox.create(c)
l2 = Label.create(c)
l2.setText("Color:")
t2 = TextBox.create(c)
t2.setUIEventCommand("c2.setFillColor(Color('#%value%'))")

c2 = Container.create(ContainerLayout.LayoutFree, c)
c2.setSize(Vector2(100,100))
c2.setAutosize(False)
c2.setFillEnabled(True)
c2.setFillColor(Color('black'))

c.requestLayoutRefresh()
