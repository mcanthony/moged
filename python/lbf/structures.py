import struct
import lbf
import itertools

def parseStringTable( node ):
    count = struct.unpack_from("i", node.payload)[0]
    table_entry_size = struct.calcsize("ii")
    table_size = table_entry_size*count
    table_part = node.payload[struct.calcsize("i"):struct.calcsize("i")+table_size]
    result = []
    table_values = [ struct.unpack_from("ii", table_part[i:i+table_entry_size]) for i in range(0,table_size,table_entry_size) ]
    for pos,length in table_values:
        result.append( node.payload[pos:pos+length] )
    return result

def writeStringTable( strings ):
    count = len(strings)
    table_size = struct.calcsize("i") + count * struct.calcsize("ii")
    curpos = table_size    
    headers = []
    string_part = []

    for string in strings:
        length = len(string)
        headers.append(curpos)
        headers.append(length)
        curpos += length+1
        string_part.append( struct.pack( str(length) + "sc", string, '\0' ) )

    return struct.pack("i" + count * "ii", count, *headers) + ''.join(string_part)
    
class Skeleton:
    def __init__(self, node = None):
        self.name = ''
        self.num_joints = 0
        self.root_offset = ()
        self.root_rotation = ()
        self.offsets = []        
        self.names = []
        self.parents = []

        if node:
            self.__parse(node)
    
    def __parse(self, node):
        header = struct.unpack_from("ifffffff", node.payload)
        self.num_joints = header[0] 
        self.root_offset = header[1:4]
        self.root_rotation = header[4:8]
                                 
        # read node.payload
        skelName = node.find( 'SKELETON_NAME' )
        if skelName:
            self.name = struct.unpack_from( str(len(skelName.payload)) + "s", skelName.payload )[0]

        skelOffsets = node.find( 'SKELETON_TRANSLATIONS' )
        if skelOffsets:
            offsets = struct.unpack_from( self.num_joints * "fff", skelOffsets.payload )
            self.offsets = [ offsets[i:i+3] for i in range(0, len(offsets), 3) ]
            
        skelNames = node.find( 'SKELETON_NAMES' )
        if skelNames:
            self.names = parseStringTable( skelNames )

        skelParents = node.find( 'SKELETON_PARENTS' )
        if skelParents:
            self.parents = struct.unpack_from( self.num_joints * "i", skelParents.payload )

    def toNode(self):
        node = lbf.LBFNode( 'SKELETON' )
        node.payload = struct.pack("ifffffff", self.num_joints,
                                   *list(itertools.chain(self.root_offset, self.root_rotation)))
        
        nameNode = node.add_child( lbf.LBFNode('SKELETON_NAME') )
        nameNode.payload = struct.pack( str(len(self.name)) + "s", self.name )

        offsetsNode = node.add_child( lbf.LBFNode('SKELETON_TRANSLATIONS'))
        offsetsNode.payload = struct.pack( self.num_joints * "fff", *list(itertools.chain(*self.offsets)))

        jointNamesNode = node.add_child( lbf.LBFNode('SKELETON_NAMES'))
        jointNamesNode.payload = writeStringTable( self.names )

        parentsNode = node.add_child( lbf.LBFNode('SKELETON_PARENTS'))
        parentsNode.payload = struct.pack( self.num_joints * "i", *self.parents)

        return node
        

class Mesh:
    def __init__(self, node = None):
        self.name = ""
        self.transform = [ [1.0, 0.0, 0.0, 0.0],
                           [0.0, 1.0, 0.0, 0.0],
                           [0.0, 0.0, 1.0, 0.0],
                           [0.0, 0.0, 0.0, 1.0] ]
        self.vertex_format = []
        self.trimesh_indices = []
        self.quadmesh_indices = []
        self.positions = []
        self.normals = []
        self.weights = []
        self.skinmats = []
        self.texcoords = []
        self.bind_rotations = []
        
        self._fmt_dict = {'POSITIONS' : (self.positions,"fff"),
                          'NORMALS' : (self.normals,"fff"),
                          'WEIGHTS' : (self.weights,"ffff"),
                          'SKINMATS' : (self.skinmats,"BBBB"),
                          'TEXCOORDS' :( self.texcoords,"ff")}
        
        if node:
            self.__parse(node)

    def __parse(self, node):
        transformValues = struct.unpack_from( "16f", node.payload )
        self.transform = [ transformValues[r:r+4] for r in range(0,16,4) ]
        
        nameNode = node.find('GEOM3D_NAME')
        if nameNode:
            self.name = struct.unpack_from( str(len(nameNode.payload)) + "s", nameNode.payload)[0]

        fmtNode = node.find('VTXFMT')
        if not fmtNode:
            return

        fmt_size = struct.unpack_from( "i", fmtNode.payload )[0]
        fmt = struct.unpack_from("i" * fmt_size, fmtNode.payload[struct.calcsize("i"):])
        self.vertex_format = map(lbf.lbf_type_to_str, fmt)
        
        triNode = node.find('TRIMESH_INDICES')
        if triNode:
            numIndices = len(triNode.payload) / struct.calcsize("I")
            self.trimesh_indices = struct.unpack_from( str(numIndices) + "I", triNode.payload)
            if ( len(self.trimesh_indices) % (fmt_size * 3) ) != 0:
                raise lbf.LBFError("Malformed trimesh index buffer: size is %d, should be a multiple of %d (3*%d)" % (len(self.trimesh_indices), 3*fmt_size, fmt_size))
            
        quadNode = node.find('QUADMESH_INDICES')
        if quadNode:
            numIndices = len(quadNode.payload) / struct.calcsize("I")
            self.quadmesh_indices = struct.unpack_from( str(numIndices) + "I", quadNode.payload)
            if ( len(self.quadmesh_indices) % (fmt_size * 4) ) != 0:
                raise lbf.LBFError("Malformed quadmesh index buffer: size is %d, should be a multiple of %d (4*%d)" % (len(self.quadmesh_indices), 4*fmt_size, fmt_size))
            
        # only load stuff specified in the format
        for fmtName in self.vertex_format:
            dataNode = node.find(fmtName)
            if dataNode:
                fmtData = self._fmt_dict[fmtName]
                size = len(dataNode.payload) / struct.calcsize(fmtData[1])
                values = struct.unpack_from(size * fmtData[1], dataNode.payload)
                packedValues = [ values[r:r+len(fmtData[1])] for r in range(0,len(values),len(fmtData[1]))]
                fmtData[0].extend(packedValues)
            else:
                raise lbf.LBFError("Missing data specified in format: %s" % (fmtName))

        bindNode = node.find('BIND_ROTATIONS')
        if bindNode:
            numValues = len(bindNode.payload) / struct.calcsize("ffff")
            values = struct.unpack_from( numValues * "ffff", bindNode.payload)
            self.bind_rotations = [values[r:r+4] for r in range(0,len(values),4)]
            
    def __verify_index_buffer(self, buffer, numVerts):
        vertStride = len(self.vertex_format)
        stride = vertStride * numVerts
        if (len(buffer) % stride) != 0:
            raise lbf.LBFError("buffer is len %d, should be a multiple of %d given the fmt" % (len(buffer), stride))

        for face in range(0,len(buffer),stride):
            for vert in range(face,face+stride,vertStride):
                for i,fmt in enumerate(self.vertex_format):
                    if i < 0 or i >= len(self._fmt_dict[fmt][0]):
                        raise lbf.LBFError("Index %d out of bounds for %s (size %d)" % (i, fmt, len(self._fmt_dict[fmt][0])))
        
    def toNode(self):
        node = lbf.LBFNode('GEOM3D')
        node.payload = struct.pack( "16f", *list(itertools.chain(*self.transform)))
        
        nameNode = node.add_child( lbf.LBFNode('GEOM3D_NAME') )
        nameNode.payload = struct.pack( str(len(self.name)) + "s", self.name )

        fmtNode = node.add_child( lbf.LBFNode('VTXFMT') )
        vertStride = len(self.vertex_format)
        fmtNumbers = map(lbf.lbf_str_to_type, self.vertex_format)
        fmtNode.payload = struct.pack("i" + vertStride * "i", vertStride, *fmtNumbers)

        nodeDict = dict.fromkeys(self._fmt_dict)
        for fmt in self.vertex_format:
            if fmt in self._fmt_dict:
                if nodeDict[fmt] != None:
                    raise lbf.LBFError("Duplicate entries in vertex format for %s" % (fmt))
                else:
                    nodeDict[fmt] = node.add_child(lbf.LBFNode(fmt))
                    nodeDict[fmt].payload = struct.pack( len(self._fmt_dict[fmt][0]) * self._fmt_dict[fmt][1],
                                                         *list(itertools.chain(*self._fmt_dict[fmt][0])))
            else:
                raise lbf.LBFError("Unknown fmt %s" % (fmt))
        
        if len(self.trimesh_indices) > 0:
            triNode = node.add_child(lbf.LBFNode('TRIMESH_INDICES'))
            self.__verify_index_buffer(self.trimesh_indices, 3)
            triNode.payload = struct.pack( len(self.trimesh_indices) * "I", *self.trimesh_indices)
        
        if len(self.quadmesh_indices) > 0:
            quadNode = node.add_child(lbf.LBFNode('QUADMESH_INDICES'))
            self.__verify_index_buffer(self.quadmesh_indices, 4)
            quadNode.payload = struct.pack( len(self.quadmesh_indices) * "I", *self.quadmesh_indices)

        if len(self.bind_rotations) > 0:
            rotNode = node.add_child(lbf.LBFNode('BIND_ROTATIONS'))
            rotNode.payload = struct.pack( len(self.bind_rotations) * "ffff", *list(itertools.chain(*self.bind_rotations)))
        
        return node
                                  
