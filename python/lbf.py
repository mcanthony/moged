#!/usr/bin/env python

# lbf reader/writer for python
import io
import sys
import getopt
import os
import os.path
import struct

lbf_type_to_str_table = {  
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

def lbf_type_to_str(typenum):
    global lbf_type_to_str_table
    if typenum in lbf_type_to_str_table:
        return lbf_type_to_str_table[typenum]
    return '(unrecognized type: %x)' % (typenum)

class LBFError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class LBFNode:
    def __init__(self, node_type, node_id, payload, first_child):
        self.typenum = node_type
        self.id = node_id
        self.payload = payload
        self.first_child = first_child
        self.next = None

class LBFFile:
    def __init__(self, f):
        self.major_version = 0
        self.minor_version = 0
        self.first_node = None

        size = 0
        f.seek(0,os.SEEK_END)
        size = f.tell()
        f.seek(0,os.SEEK_SET)

        self.__parse_header(f)
        self.first_node = self.__parse_chunk(f, size - f.tell())

    def __parse_header(self, f):
        try:
            header_fmt = "4shh8x"
            header_data = f.read(struct.calcsize(header_fmt))
            header = struct.unpack_from(header_fmt, header_data)
            if( header[0] != "LBF_" ):
                raise LBFError("Missing LBF file magic")
            self.major_version = header[1]
            self.minor_version = header[2]
        except struct.error as err:
            raise LBFError("Failed to parse LBF header:\n" + str(err))
    
    def __parse_chunk(self, f, size_left):
        try:
            firstNode = None
            insertPoint = None
            while size_left > 0:
                curpos = f.tell()

                chunk_header_fmt = "iiii"
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
                    first_child = self.__parse_chunk(f, children_size)
            
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
        

def parseLBF(fname):
    with open(fname,"rb") as f:
        lbf = LBFFile(f)
        return lbf
    return None

def usage():
    print sys.argv[0], "--file lbffile --ls"

def list_neighbors(node, indent = 0):
    while node:
        prefix = ''
        if indent > 0:
            prefix = '|----'* (indent - 1) + '|---' + '>'
            
        print prefix + lbf_type_to_str(node.typenum) + "(" + str(node.id) + ") + " + str(len(node.payload))
        list_neighbors(node.first_child, indent + 1)
        node = node.next

def main():
    lbffile = ''
    mode = 'list'

    try:
        opts,args = getopt.getopt(sys.argv[1:], "f:", ["file=","ls"])
        for opt,val in opts:
            if opt in ('-f','--file'):
                lbffile = val
            elif opt == '--ls':
                mode = 'list'
    except getopt.GetoptError:
        usage()
        sys.exit(1)

    if not os.path.exists(lbffile):
        print lbffile, "does not exist"
        sys.exit(2)


    if mode == 'list':
        try:
            print "Loading",lbffile
            lbf = parseLBF(lbffile)
            print "LBF File Version:",str(lbf.major_version) + "." + str(lbf.minor_version)
            list_neighbors(lbf.first_node)
        except LBFError as err:
            print "Error occurred:\n",err

if __name__ == "__main__":
    main()

