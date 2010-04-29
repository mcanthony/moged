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
        

