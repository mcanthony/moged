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
    ob.makeDisplayList()
    Blender.Redraw()
    
    return ob

def _import_anim_section(node):
    skelNode = node.find( 'SKELETON' )
    if(skelNode):
        return _import_skeleton(skelNode)
    return None

def _import_faces(me, buf, mesh, num_verts):
    faces = [ buf[r:r+num_verts] for r in range(0,len(buf),num_verts)]
    first_face = len(me.faces)
    me.faces.extend(faces)
    if 'TEXCOORDS' in mesh.vertex_format:
        tex_off = mesh.vertex_format.index('TEXCOORDS')
        for i,face_indices in enumerate(faces):
            face = first_face + i
            texcoords = map(lambda x: Vector(mesh.verts[x][tex_off][0],mesh.verts[x][tex_off][1],0), face_indices)
            me.faces[face].uv = texcoords

# todo: move to common place, this is too similar to lbf_export.py's ordering function
def _get_export_ordered_bones(bone_list):
    working = []
    for bone in bone_list:
        if bone.parent==None:
            working.append(bone)    
    result = []
    working = sorted(working, key=lambda bone: bone.name, reverse=True)
    while len(working)>0:
        cur = working.pop()
        for child in sorted(cur.children, key=lambda bone: bone.name, reverse=True):
            working.append(child)
        result.append(cur)
    return result

def _import_geom3d(node, skelObj):
    mesh = lbf.structures.Mesh(node)
    me = bpy.data.meshes.new(mesh.name)

    pos_off = mesh.vertex_format.index('POSITIONS')
    me.verts.extend(map(lambda v: v[pos_off], mesh.verts))

    if 'NORMALS' in mesh.vertex_format:
        off = mesh.vertex_format.index('NORMALS')
        for i,vert in enumerate(me.verts):
            vert.no = Vector(mesh.verts[i][off])

    if 'TEXCOORDS' in mesh.vertex_format:
        me.addUVLayer('texcoords')
        me.activeUVLayer = 'texcoords'
        
    _import_faces(me, mesh.trimesh_indices, mesh, 3)
    _import_faces(me, mesh.quadmesh_indices, mesh, 4)

    scn = bpy.data.scenes.active
    ob = scn.objects.new(me,mesh.name)
    mat = Matrix( mesh.transform[0], 
                  mesh.transform[1],
                  mesh.transform[2],
                  mesh.transform[3] ).transpose()
    ob.setMatrix(mat)

    ## TODO  make sure this owrks with new format (specifically weights etc)
    # weight assignment has to be after object linking
    if skelObj and 'WEIGHTS' in mesh.vertex_format and 'SKINMATS' in mesh.vertex_format:
        skelData = skelObj.getData()
        bones = _get_export_ordered_bones(skelData.bones.values())
        for bone in bones:
            me.addVertGroup(bone.name)
        
        weight_offset = mesh.vertex_format.index('WEIGHTS')
        skin_offset = mesh.vertex_format.index('SKINMATS')

        for i,vert in enumerate(me.verts):
            weights = mesh.verts[i][weight_offset]
            skins = mesh.verts[i][skin_offset]
            skinnames = map(lambda idx: bones[idx].name, skins)            
            for skin,weight in zip(skinnames,weights):
                me.assignVertsToGroup( skin, [i], weight, Blender.Mesh.AssignModes.ADD)
        mod = ob.modifiers.append( Blender.Modifier.Types.ARMATURE )
        mod[Blender.Modifier.Settings.OBJECT] = skelObj
        print mod[Blender.Modifier.Settings.OBJECT] 
        mod[Blender.Modifier.Settings.OBJECT]  = mod[Blender.Modifier.Settings.OBJECT] 
        mod[Blender.Modifier.Settings.VGROUPS] = True
        ob.makeDisplayList()

    Blender.Redraw()

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
    Blender.Window.RedrawAll()

if __name__ == '__main__':
    Blender.Window.FileSelector(import_lbf, 'Import LBF', '*.lbf')

