from contextlib import contextmanager

import ctypes
import hashlib
import re
import struct
import uuid
import os

import dbgHelp
from logger import logger
from trace_info import TraceInfo


def md5_to_uuid(hash_result):
    """
    Convert an md hash to a UUID3 value the same way as wpp-ng c++ code.
    """
    parts = struct.unpack_from('<IHH2s6s', hash_result)
    result = struct.pack('>IHH2s6s', parts[0], parts[1], (parts[2] & 0x0fff) | 0x3000, parts[3], parts[4])
    return uuid.UUID(bytes=result)


def md5_ints_to_digest(a, b, c, d):
    """
    Combine four uint32_t values into an md5 message digest.
    """
    return struct.pack('<IIII', a, b, c, d)


def parse_int(arg):
    """
    Parse an integer of an unknown base.
    The supported bases are 2, 8, 10 and 16.
    """
    arg = arg.lower()
    base = 10
    if arg.startswith('0x'):
        base = 16
    elif arg.startswith('0b'):
        base = 2
    elif arg.startswith('0o'):
        base = 8
    
    return int(arg, base)


def split_args(args):
    """
    Split template arguments, taking into consideration brackets of the given types: [], (), {}, <>.
    """
    return re.split(r'\s*,\s*(?![^{]*})(?![^(]*\))(?![^<]*\>)(?![^\[]*\])', args)


def parse_annotation(psymbol_info, symbol_size, user_context):
    """
    Parse the given annotation information taken from the pdb file.
    """
    info, types = user_context
    
    symbol_info = psymbol_info[0]
    name_pointer = ctypes.cast(symbol_info.Name, ctypes.POINTER(ctypes.c_char * symbol_info.NameLen))
    name = bytearray(name_pointer[0]).decode()
    trace_data = name.split('\x00')
    # Since the data ends with double '\x00', there is an empty string left in the list - remove it.
    trace_data.pop()
    if trace_data[0] == 'TMF_NG:':
        # Primary trace info, containing the path, function, line, flag, level and arguments.
        # Get file directory and name for the hash (and not full path)
        file_path = trace_data[1]
        file_path = os.path.join(os.path.basename(os.path.dirname(file_path)), os.path.basename(file_path))
        trace_data[1] = file_path
        
        # Now calculate the trace hash
        md5_hash = hashlib.md5(''.join(trace_data).encode()).digest()
        guid = md5_to_uuid(md5_hash)

        if guid in info:
            raise Exception('GUID {} found twice!'.format(guid))

        info[guid] = trace_data[1:]  # No need for the "TMF_NG:" part
    elif trace_data[0] == 'TMF_NG_TYPES:':
        # Secondary trace info, containing the types of the arguments.
        
        args = trace_data[1]
        assert 'annotateArgTypes<' in args
        
        # The data format is: `...annotateArgTypes<TraceItem1, TraceItem2, ...>` - extract the trace item types
        args = args[args.index('<') + 1:args.rindex('>')]
        args = split_args(args)
        
        # Calculate the message GUID. It should eventually match some primary info message hash.
        hash_parts = [parse_int(arg) for arg in args[:4]]
        md5_hash = md5_ints_to_digest(*hash_parts)
        guid = md5_to_uuid(md5_hash)

        if guid in types:
            raise Exception('GUID {} found twice in types!'.format(guid))

        types[guid] = args[4:]  # No need to store the hash
    else:
        logger.warning('Unexpected annotation data: {}!'.format(name.encode('hex')))
        logger.warning('Skipping...')
    
    # Continue to the next annotation
    return True


@contextmanager
def initialize_dbghelp():
    """
    Initialize DbgHelp.dll for symbol parsing, and yield a usable dbghelp handle. 
    """
    handle = ctypes.windll.kernel32.GetCurrentThreadId()
    dbgHelp.SymSetOptions(dbgHelp.SymOpt.CASE_INSENSITIVE, dbgHelp.SymOpt.UNDNAME, dbgHelp.SymOpt.LOAD_LINES, dbgHelp.SymOpt.ALLOW_ABSOLUTE_SYMBOLS, dbgHelp.SymOpt.AUTO_PUBLICS)
    dbgHelp.SymInitialize(handle, None, False)
    try:
        yield handle
    finally:
        dbgHelp.SymCleanup(handle)


@contextmanager
def load_module(handle, pdb_path):
    """
    Load the given pdb path.
    """
    pdb_path = pdb_path.encode()
    module_name = os.path.basename(pdb_path)
    base_address = dbgHelp.SymLoadModuleEx(handle, 0, pdb_path, module_name, dbgHelp.DEFAULT_BASE_ADDRESS, os.path.getsize(pdb_path), 0, 0)

    try:
        yield base_address
    finally:
        dbgHelp.SymUnloadModule64(handle, base_address)


def extrace_trace_info(pdb_path):
    """
    Extract trace information from the annotations in the given PDB file.

    :type pdb_path: str
    """
    info = {}  # map GUID to general info
    types = {}  # map GUID to argument type info
    with initialize_dbghelp() as handle:
        with load_module(handle, pdb_path) as base_address:
            context = [info, types]
            dbgHelp.SymSearch(handle, base_address, 0, dbgHelp.SymTagEnum.Annotation, 0, 0, parse_annotation, context, dbgHelp.SymSearchOpt.RECURSE)

    # Assert primary and secondary was found for all the GUIDs
    assert set(info) == set(types), 'Bad annotations!'

    return [TraceInfo(guid, info[guid], types[guid]) for guid in info.keys()]