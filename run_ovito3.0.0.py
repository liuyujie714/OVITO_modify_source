#!/usr/bin/python3
# coding: utf-8

import os
os.environ['OVITO_GUI_MODE'] = '1'

import ovito
from ovito.io import import_file, export_file
from ovito.vis import Viewport, OpenGLRenderer
from ovito.modifiers import SelectTypeModifier, \
                            CreateBondsModifier, \
                            ColorCodingModifier, \
                            AffineTransformationModifier, \
                            PythonScriptModifier
from ovito.data import SimulationCell

class Render:
    def __init__(self, fname, **kwargs) -> None:
        self.pipeline = import_file(fname, **kwargs)
    
    def render(self):
        # rotate system
        self.pipeline.modifiers.append(AffineTransformationModifier(
            operate_on = {'particles', 'cell'},
            transformation =[[0,  0, 1, 0],
                             [0,  1, 0, 0],
                             [-1, 0, 0, 0]]
        ))

        # simulation cell setup
        cell = self.pipeline.compute().expect(SimulationCell)
        cell.vis.rendering_color = (0.0, 1.0, 0.0)
        cell.pbc = (True, True, True)

        # add trajectory to scene
        self.pipeline.add_to_scene()

        # add selection
        self.pipeline.modifiers.append(SelectTypeModifier(
            operate_on='particles',
            property='Particle Type',
            types = {'CH4', 'O'}
        ))

        # create bonds
        bondmodify = CreateBondsModifier(
            mode=CreateBondsModifier.Mode.Uniform
        )
        pairs_cutoff = {
            ('CH4', 'CH4') : 1.5,
            ('CH4', 'O')   : 1.5,
            ('O', 'O')     : 1.5,
        }
        for key in pairs_cutoff:
            bondmodify.set_pairwise_cutoff(key[0], key[1], pairs_cutoff[key])
        bondmodify.vis.width = 0.2
        self.pipeline.modifiers.append(bondmodify)

        # set particle vdw radius
        def modify(frame, data, output):
            types = data.particle_properties.particle_type
            types.type_by_name('CH4').radius = 0.425
            types.type_by_name('O').radius = 0.3
            types.type_by_name('H').radius = 0.15
            types.type_by_name('MW').radius = 0.0000001
        self.pipeline.modifiers.append(PythonScriptModifier(function=modify))

        self.pipeline.modifiers.append(ColorCodingModifier(
            operate_on='particles',
            property='Molecule Type',
            gradient = ColorCodingModifier.Hot(),
            only_selected = True
        ))

        
        # set render view
        vp = Viewport(type=Viewport.Type.Ortho, 
                      camera_dir = (0, 1, 0) # Y axis
                      )
        vp.zoom_all()
        vp.render_anim(filename=r'C:\Users\liuyujie714\Desktop\video.mp4', 
                       size=(1920, 1080),
                       background=(1,1,1), # white
                       every_nth=10,
                       renderer=OpenGLRenderer()
                       )
    
    def save(self, filename):
        ovito.scene.save(filename)
        
        

obj = Render(fname=r'C:\Users\liuyujie714\Desktop\CalCH4\x64\Release\XXX.xyz',
             columns=['Particle Type', 
                      'Position.X', 'Position.Y', 'Position.Z', 
                      'Molecule Type'])
obj.render()
obj.save(r'C:\Users\liuyujie714\Desktop\xxx.ovito')


