#!BPY
"""
Name: 'Luke/Lab Binary File Format (.lbf)...'
Blender: 249
Group: 'Export'
Tooltip: 'Export LBF file format (*.lbf)'
"""

__author__ = "Lucas Panian"
__url__ = ("Author's Homepage, http://panian.net")
__version__ = "1.0"

import Blender
from Blender import Armature
from Blender.Mathutils import *
import Blender.Draw
import lbf
import lbf.structures
import os
import os.path

def to_lbf_quat( quat ):
    return (quat[1],quat[2],quat[3],quat[0])

def _export_skeleton(ob, node):
    data = ob.getData()
    obMat = ob.getMatrix()
    
    skel = lbf.structures.Skeleton()
    skel.name = data.name
    skel.root_offset = obMat.translationPart()
    skel.root_rotation = to_lbf_quat(obMat.rotationPart().toQuat())
    
    working_bones = []
    for bone in data.bones.values():
        if bone.parent == None:
            working_bones.append((bone,-1))

    working_bones = sorted(working_bones, key=lambda bone: bone[0].name, reverse=True)
    parents = []
    while len(working_bones) > 0:
        cur,p_index = working_bones.pop()
        my_index = len(parents)
        parents.append(p_index)        

        for child in sorted(cur.children, key=lambda bone: bone.name, reverse=True):
            working_bones.append((child,my_index))
        offset = cur.tail['ARMATURESPACE'] - cur.head['ARMATURESPACE']
        skel.offsets.append(list(offset))
        skel.names.append( cur.name )

    for i in range(0,len(parents)):
        parent = parents[i]
        pname ="none"
        if parent >= 0:
            pname = skel.names[parent]
    
    skel.parents = parents
    skel.num_joints = len(parents)
    node.add_child( skel.toNode() )

def _export_mesh(ob, node):
    data = ob.getData(False, True)
    print ob

def export_lbf(fname):
    if not fname.lower().endswith('.lbf'):
        fname += '.lbf'

    # are you SUUUURE?? 
    if os.path.exists(fname):
        result = Blender.Draw.PupMenu("Save Over?%t|Yes|No")
        if result != 1:
            return

    Blender.Window.WaitCursor(1)
    sce = Blender.Scene.GetCurrent()

    outlbf = lbf.LBFFile()
    objSection = outlbf.add_node( lbf.LBFNode('OBJECT_SECTION') )
    animSection = outlbf.add_node( lbf.LBFNode('ANIM_SECTION') )

    for ob in sce.objects:
        if ob.type == 'Armature':
            _export_skeleton(ob, animSection)
        elif ob.type == 'Mesh':
            _export_mesh(ob, objSection)

    lbf.writeLBF(outlbf, fname)

    Blender.Window.WaitCursor(0)
if __name__ == '__main__':
    Blender.Window.FileSelector(export_lbf, 'Export LBF', '*.lbf')
