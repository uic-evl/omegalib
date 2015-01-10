from omegaVtk import *
from sceneTools import spin_navigation

vtkModule = VtkModule.createAndInitialize()

import vtk

reader = vtk.vtkPolyDataReader()
reader.SetFileName(ofindFile("./track90.vtk"))

mapper = vtk.vtkPolyDataMapper()
mapper.SetInputConnection(reader.GetOutputPort())

actor = vtk.vtkActor()
actor.SetMapper(mapper)

vtkNode = SceneNode.create("vtkNode")
vtkNode.setPosition(Vector3(0, 1, -10))
vtkNode.setScale(0.05,0.05,0.05)
spin_navigation.root = vtkNode

vtkAttachProp(actor, vtkNode)

l = vtk.vtkLight()
vtkAddLight(l)
#l.PositionalOn()
l.SetPosition(0, 4, 0)
l.SetColor(1,1,1)

#c = getDefaultCamera()
#c.focusOn(vtkNode)