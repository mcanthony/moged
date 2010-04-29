#!/usr/bin/env python
__all__ = ["structures"] 

# lbf reader/writer for python
import io
import sys
import os
import os.path
import struct

_lbf_type_to_str_table = {  
    0x0500 : 'OBJECT_SECTION',
    0x0501 : 'ANIM_SECTION',
    0x1000 : 'GEOM3D',
    0x1001 : 'GEOM3D_NAME',
    0x1050 : 'VTXFMT',
    0x1100 : 'TRIMESH_INDICES',
    0x1101 : 'QUADMESH_INDICES',
    0x1200 : 'POSITIONS',
    0x1201 : 'NORMALS',
    0x1202 : 'WEIGHTS',
    0x1203 : 'SKINMATS',
    0x1204 : 'BIND_ROTATIONS',
    0x1205 : 'TEXCOORDS',
    0x2000 : 'ANIMATION',
    0x2101 : 'ANIMATION_NAME',
    0x2202 : 'FRAME',
    0x2203 : 'FRAME_ROTATIONS',
    0x2204 : 'FRAME_ROOT_OFFSETS',
    0x2205 : 'FRAME_ROOT_ROTATIONS',
    0x3000 : 'SKELETON',
    0x3001 : 'SKELETON_NAME',
    0x3002 : 'SKELETON_TRANSLATIONS',
    0x3003 : 'SKELETON_ROTATIONS',
    0x3004 : 'SKELETON_NAMES',
    0x3005 : 'SKELETON_PARENTS',
}

_lbf_str_to_type_table = {
    'OBJECT_SECTION' : 0x0500,
    'ANIM_SECTION' : 0x0501,
    'GEOM3D' : 0x1000,
    'GEOM3D_NAME' : 0x1001,
    'VTXFMT' : 0x1050,
    'TRIMESH_INDICES' : 0x1100,
    'QUADMESH_INDICES' : 0x1101,
    'POSITIONS' : 0x1200,
    'NORMALS' : 0x1201,
    'WEIGHTS' : 0x1202,
    'SKINMATS' : 0x1203,
    'BIND_ROTATIONS' : 0x1204,
    'TEXCOORDS' : 0x1205,
    'ANIMATION' : 0x2000,
    'ANIMATION_NAME' : 0x2101,
    'FRAME' : 0x2202,
    'FRAME_ROTATIONS' : 0x2203,
    'FRAME_ROOT_OFFSETS' : 0x2204,
    'FRAME_ROOT_ROTATIONS' : 0x2205,
    'SKELETON' : 0x3000,
    'SKELETON_NAME' : 0x3001,
    'SKELETON_TRANSLATIONS' : 0x3002,
    'SKELETON_ROTATIONS' : 0x3003,
    'SKELETON_NAMES' : 0x3004,
    'SKELETON_PARENTS' : 0x3005 
}

header_fmt = "4shh8x"
chunk_header_fmt = "iiii"
lbf_version = (1,0)

# todo: use python lists to hold children - don't need to store 'next' the same way as is done
# in the file!

def lbf_type_to_str(typenum):
    global _lbf_type_to_str_table
    if typenum in _lbf_type_to_str_table:
        return _lbf_type_to_str_table[typenum]
    return '(unrecognized type: %x)' % (typenum)

def lbf_str_to_type(name):
    global _lbf_str_to_type_table
    if name in _lbf_str_to_type_table:
        return _lbf_str_to_type_table[name]
    return -1

class LBFError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class LBFNode:
    def __init__(self, node_type, node_id = 0, payload = '', first_child = None):
        if type(node_type) == str:
            num_node_type = lbf_str_to_type(node_type)
            if num_node_type == -1:
                raise LBFError("Unrecognized type %s" % (node_type))
            node_type = num_node_type
        self.typenum = node_type
        self.id = node_id
        self.payload = payload
        self.first_child = first_child
        self.next = None
        # cached_size only used when writing to prevent recomputing the size of the node
        self.cached_size = 0

    def find( self, type_name ):
        typenum = lbf_str_to_type(type_name)
        if(typenum == -1):
            return None

        curNode = self.first_child
        while curNode:
            if curNode.typenum == typenum:
                return curNode
            curNode = curNode.next
        return None

    def add_child( self, newNode ):
        if self.first_child:
            self.first_child.add_sibling(newNode)
        else:
            self.first_child = newNode
        return newNode

    def add_sibling( self, newNode ):
        if self.next:
            cur = self.next
            while cur.next:
                cur = cur.next
            cur.next = newNode
        else:
            self.next = newNode
        return newNode

def _cacheNodeLength(node):
    childrenSize = 0
    childNode = node.first_child
    while childNode:
        _cacheNodeLength(childNode)
        childrenSize +=  childNode.cached_size
        childNode = childNode.next

    global chunk_header_fmt
    node.cached_size = childrenSize + len(node.payload) + struct.calcsize(chunk_header_fmt)
    

# requires that node sizes be cached in node.cached_size
def _writeNode(f, node):
    global chunk_header_fmt
    children_offset = len(node.payload) + struct.calcsize(chunk_header_fmt)
    chunk_header = (node.typenum, node.cached_size, children_offset, node.id)
    chunk_header_data = struct.pack(chunk_header_fmt, *chunk_header)
    f.write(chunk_header_data)
    f.write(node.payload)

    child = node.first_child
    while child:
        _writeNode(f, child)
        child = child.next

def _parseChunk(f, size_left):
    global chunk_header_fmt 
    try:
        firstNode = None
        insertPoint = None
        while size_left > 0:
            curpos = f.tell()

            chunk_header_data = f.read(struct.calcsize(chunk_header_fmt))
            chunk_header = struct.unpack_from(chunk_header_fmt, chunk_header_data)

            node_type = chunk_header[0]
            length = chunk_header[1]
            child_offset = chunk_header[2]
            node_id = chunk_header[3]

            payload_size = child_offset - struct.calcsize(chunk_header_fmt)
            payload_data = f.read(payload_size)

            children_size = length - child_offset
            first_child = None
            if children_size > 0:
                first_child = _parseChunk(f, children_size)
            
            next_chunk_start = curpos + length
            f.seek(next_chunk_start, os.SEEK_SET)
            size_left = size_left - length

            newNode = LBFNode(node_type, node_id, payload_data, first_child)
            if firstNode:
                insertPoint.next = newNode
            else:
                firstNode = newNode
            insertPoint = newNode
        return firstNode        
    except struct.error as err:
        raise LBFError("Failed to parse chunk header:\n" + str(err))

class LBFFile:
    def __init__(self):
        self.major_version = 0
        self.minor_version = 0
        self.first_node = None

    def parseFile(self, f):
        try:
            size = 0
            f.seek(0,os.SEEK_END)
            size = f.tell()
            f.seek(0,os.SEEK_SET)

            global header_fmt
            header_data = f.read(struct.calcsize(header_fmt))
            header = struct.unpack_from(header_fmt, header_data)
            if( header[0] != "LBF_" ):
                raise LBFError("Missing LBF file magic")

            self.major_version = header[1]
            self.minor_version = header[2]

            global lbf_version
            if self.major_version != lbf_version[0]:
                raise LBFError("Cannot load major version %d, only %d supported" % (self.major_version, lbf_version[0]))
            if self.minor_version > lbf_version[1]:
                raise LBFError("Cannot load minor version %d, only up to %d supported" % (self.minor_version, lbf_version[1]))

            self.first_node = _parseChunk(f, size - f.tell())
        except struct.error as err:
            raise LBFError("Failed to parse LBF header:\n" + str(err))
    
    def writeToFile(self, f):
        global lbf_version
        global header_fmt
        header = ('LBF_', lbf_version[0], lbf_version[1])
        header_data = struct.pack(header_fmt, *header)
        f.write(header_data)
        
        # precompute node sizes so each level of the tree doesn't have to compute it
        curNode = self.first_node
        while curNode:
            _cacheNodeLength(curNode)
            curNode = curNode.next

        curNode = self.first_node
        while curNode:
            _writeNode(f, curNode)
            curNode = curNode.next

    def find( self, type_name ):
        typenum = lbf_str_to_type(type_name)
        if(typenum == -1):
            return None

        curNode = self.first_node
        while curNode:
            if curNode.typenum == typenum:
                return curNode
            curNode = curNode.next
        return None

    def add_node( self, newNode):
        if self.first_node:
            curNode = self.first_node
            while curNode.next:
                curNode = curNode.next
            curNode.next = newNode
        else:
            self.first_node = newNode
        return newNode

def parseLBF(fname):
    with open(fname,"rb") as f:
        lbf = LBFFile()
        lbf.parseFile(f)
        return lbf
    return None

def writeLBF(lbf, fname):
    with open(fname, "wb") as f:
        lbf.writeToFile(f)

def list_neighbors(node, indent = 0):
    while node:
        prefix = ''
        if indent > 0:
            prefix = '|----'* (indent - 1) + '|---' + '>'
            
        print prefix + lbf_type_to_str(node.typenum) + "(" + str(node.id) + ") + " + str(len(node.payload))
        list_neighbors(node.first_child, indent + 1)
        node = node.next


