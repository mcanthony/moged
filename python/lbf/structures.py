import struct

# TODO move to lbf.structures
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
    
class Skeleton:
    def __init__(self, node = None):
        self.name = ''
        self.num_joints = 0
        self.root_offset = ()
        self.root_rotation = ()
        self.offsets = []        
        self.rotations = []
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
            self.name = struct.unpack_from( str(len(skelName.payload)) + "s", skelName.payload )

        skelOffsets = node.find( 'SKELETON_TRANSLATIONS' )
        if skelOffsets:
            offsets = struct.unpack_from( self.num_joints * "fff", skelOffsets.payload )
            self.offsets = [ offsets[i:i+3] for i in range(0, len(offsets), 3) ]
            
        skelRotations = node.find( 'SKELETON_ROTATIONS' )
        if skelRotations:
            rots = struct.unpack_from( self.num_joints * "ffff", skelRotations.payload )
            self.rotations = [ rots[i:i+4] for i in range(0, len(rots), 4) ]

        skelNames = node.find( 'SKELETON_NAMES' )
        if skelNames:
            self.names = parseStringTable( skelNames )

        skelParents = node.find( 'SKELETON_PARENTS' )
        if skelParents:
            self.parents = struct.unpack_from( self.num_joints * "i", skelParents.payload )

