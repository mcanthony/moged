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
import bpy
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
    return ob

def _import_anim_section(node):
    skelNode = node.find( 'SKELETON' )
    if(skelNode):
        return _import_skeleton(skelNode)
    return None

def _import_faces(me, buf, mesh, num_verts):
    faces = []
    vert_stride = len(mesh.vertex_format)
    face_stride = num_verts * vert_stride
    pos_offset = mesh.vertex_format.index('POSITIONS')
    for faceIdx in range(0, len(buf), face_stride):
        faces.append(buf[faceIdx+pos_offset:faceIdx+pos_offset+face_stride:vert_stride])
    
    first_face = len(me.faces)
    me.faces.extend(faces)

    if len(mesh.texcoords) > 0:
        tex_offset = mesh.vertex_format.index('TEXCOORDS')
        for faceIdx in range(0, len(buf), face_stride):
            face = first_face + faceIdx/face_stride
            indices = buf[faceIdx+tex_offset:faceIdx+tex_offset+face_stride:vert_stride]
            texcoords = map(lambda x: Vector(mesh.texcoords[x][0],mesh.texcoords[x][1],0), indices)
            me.faces[face].uv = texcoords

def _import_geom3d(node, skelObj):
    mesh = lbf.structures.Mesh(node)
    me = bpy.data.meshes.new(mesh.name)

    me.verts.extend(mesh.positions)
    if len(mesh.normals) > 0:
        for i,vert in enumerate(me.verts):
            vert.no = Vector(mesh.normals[i])

    if len(mesh.texcoords) > 0:
        me.addUVLayer('texcoords')
        me.activeUVLayer = 'texcoords'
        
    _import_faces(me, mesh.trimesh_indices, mesh, 3)
    _import_faces(me, mesh.quadmesh_indices, mesh, 4)

    #if skelObj:
#        add weights

    scn = bpy.data.scenes.active
    ob = scn.objects.new(me,mesh.name)

def _import_obj_section(node, skeletonObj):
    geomNode = node.find('GEOM3D')
    while geomNode:
        _import_geom3d(geomNode, skeletonObj)
        geomNode = geomNode.find_next('GEOM3D')

def import_lbf(path):
    editmode = Blender.Window.EditMode()
    if editmode: Blender.Window.EditMode(0)

    Blender.Window.WaitCursor(1)
    lbf_top = lbf.parseLBF(path)
    
    skelObj = None
    animNode = lbf_top.find( 'ANIM_SECTION' )
    if animNode:
        skelObj = _import_anim_section(animNode)

    obNode = lbf_top.find( 'OBJECT_SECTION' )
    if obNode:
        _import_obj_section( obNode, skelObj )
    
    Blender.Window.WaitCursor(0)
    
    if editmode: Blender.Window.EditMode(1)

if __name__ == '__main__':
    Blender.Window.FileSelector(import_lbf, 'Import LBF', '*.lbf')

