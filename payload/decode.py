
from collections import namedtuple
from struct import unpack

telem_packet_struct_format = "!LdH"
TelemPacket = namedtuple('TelemPacket', 'timehms pi status_flags')


"""Returns TelemPacket object (namedtuple). """
def unpack_telem_packet(data:bytes):
    return TelemPacket._make(unpack(telem_packet_struct_format, data))
