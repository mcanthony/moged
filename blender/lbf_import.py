#!BPY
"""
Name: 'Luke/Lab Binary File Format (.lbf)...'
Blender: 249
Group: 'Import'
Tooltip: 'Import LBF file format (*.lbf)'
"""

__author__ = "Lucas Panian"
__url__ = ("Author's Homepage, http://panian.net")
__version__ = "1.0"

import Blender
from Blender import Armature
from Blender.Mathutils import *
import lbf
import lbf.structures

################################################################################
def _world_offset_to_blender_matrix(offset, invParentMat):
    boneOffset = offset * invParentMat
    angle = AngleBetweenVecs(boneOffset, Vector(0,1,0))
    if angle > 0.001:
        axis = boneOffset.cross(Vector(0,1,0))
        rotQ = Quaternion(axis,angle)
        rotMat = rotQ.toMatrix().resize4x4()
        rotMat.invert()
        return rotMat
    else:
        return Matrix().identity()

################################################################################

def _import_skeleton(node):
    skel = lbf.structures.Skeleton(node)
    scn = Blender.Scene.GetCurrent()

    arm = Armature.New(skel.name)
    arm.drawType = Armature.STICK
    arm.makeEditable()
    
    # blender is r,a,b,c, lbf is a,b,c,r
    root_quat = Quaternion(skel.root_rotation[3],*skel.root_rotation[0:3])
    root_offset = Vector(skel.root_offset)

    bone_objects = [None] * skel.num_joints
    for i in range(0,skel.num_joints):
        eb = Armature.Editbone()
        eb.name = skel.names[i]
        parent = skel.parents[i]

        currentOffset = Vector(skel.offsets[i])        
        if parent >= 0:
            eb.head = Vector(0,0,0) + bone_objects[parent].tail
            eb.tail = eb.head + currentOffset
            eb.parent = bone_objects[parent]
        else:
            eb.head = Vector(0,0,0)
            eb.tail = currentOffset
        
        bone_objects[i] = eb
        arm.bones[eb.name] = eb

    ob = scn.objects.new(arm)
    ob.setMatrix( TranslationMatrix(root_offset) * root_quat.toMatrix().resize4x4())
    arm.update()
    Blender.Redraw()

def _import_anim_section(node):
    skelNode = node.find( 'SKELETON' )
    if(skelNode):
        _import_skeleton(skelNode)

def import_lbf(path):
    Blender.Window.WaitCursor(1)
    lbf_top = lbf.parseLBF(path)
    
    animNode = lbf_top.find( 'ANIM_SECTION' )
    if animNode:
        _import_anim_section(animNode)
    
    Blender.Window.WaitCursor(0)

if __name__ == '__main__':
    Blender.Window.FileSelector(import_lbf, 'Import LBF', '*.lbf')

