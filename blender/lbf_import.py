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
import lbf
import lbf.structures

################################################################################

def _import_skeleton(node):
    skel = lbf.structures.Skeleton(node)
    print skel.name
    print skel.num_joints
    print skel.root_offset
    print skel.root_rotation
    print skel.offsets
    print skel.rotations
    print skel.parents
    print skel.names

def _import_anim_section(node):
    skelNode = node.find( 'SKELETON' )
    if(skelNode):
        _import_skeleton(skelNode)

def import_lbf(path):
    print path
    Blender.Window.WaitCursor(1)
    lbf_top = lbf.parseLBF(path)
    
    animNode = lbf_top.find( 'ANIM_SECTION' )
    if animNode:
        _import_anim_section(animNode)
    
    Blender.Window.WaitCursor(0)

if __name__ == '__main__':
    Blender.Window.FileSelector(import_lbf, 'Import LBF', '*.lbf')
