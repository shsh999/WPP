"""
This file contains thin wrappers for some dbghelp.dll functions.
"""
import ctypes


_DBG_HELP_DLL = ctypes.WinDLL('dbghelp.dll')

DEFAULT_BASE_ADDRESS = 0x1000000   # Base address of the PDB file that will be loaded.


class DbgHelpException(Exception):
    def __init__(self, *args, **kwargs):
        super(DbgHelpException, self).__init__(*args, **kwargs)
        self.last_error = ctypes.GetLastError()
    
    def __str__(self):
        return 'Last error: {}'.format(self.last_error)


class InitializationError(DbgHelpException):
    pass

class CleanupError(DbgHelpException):
    pass

class SearchError(DbgHelpException):
    pass

class LoadModuleError(DbgHelpException):
    pass

class UnLoadModuleError(DbgHelpException):
    pass


_SymInitialize = _DBG_HELP_DLL["SymInitialize"]
_SymCleanup = _DBG_HELP_DLL["SymCleanup"]
_SymSetOptions = _DBG_HELP_DLL["SymSetOptions"]
_SymSearch = _DBG_HELP_DLL["SymSearch"]
_SymLoadModuleEx = _DBG_HELP_DLL["SymLoadModuleEx"]
_SymUnloadModule64 = _DBG_HELP_DLL["SymUnloadModule64"]

def SymInitialize(process_handle, user_search_path=None, invade_process=False):
    if not _SymInitialize(process_handle, user_search_path, invade_process):
        raise InitializationError()


def SymCleanup(process_handle):
    if not _SymCleanup(process_handle):
        raise CleanupError()


class SymOpt:
    """
    Define enumeration that is used in SymSetOptions.
    """
    DEFERRED_LOADS = 0x00000004
    
    CASE_INSENSITIVE = 0x00000001
    UNDNAME = 0x00000002
    LOAD_LINES = 0x00000010
    ALLOW_ABSOLUTE_SYMBOLS = 0x00000800
    AUTO_PUBLICS = 0x00010000

    DEBUG = 0x80000000
    IGNORE_CVREC = 0x00000080
    EXACT_SYMBOLS = 0x00000400
    FAIL_CRITICAL_ERRORS = 0x00000200


def SymSetOptions(*arg_options):
    options = 0
    for opt in arg_options:
        options |= opt
    _SymSetOptions(options)


class SYMBOL_INFO(ctypes.Structure):
    """
    A pointer to this type of struct is passed to the handler function of the SymSearch function.
    """
    _fields_ = [
        ('SizeOfStruct', ctypes.c_ulong),
        ('TypeIndex', ctypes.c_ulong),
        ('Reserved', ctypes.c_ulonglong * 2),
        ('Index', ctypes.c_ulong),
        ('Size', ctypes.c_ulong),
        ('ModBase', ctypes.c_ulonglong),
        ('Flags', ctypes.c_ulong),
        ('Value', ctypes.c_ulonglong),
        ('Address', ctypes.c_longlong),
        ('Register', ctypes.c_ulong),
        ('Scope', ctypes.c_ulong),
        ('Tag', ctypes.c_ulong),
        ('NameLen', ctypes.c_ulong),
        ('MaxNameLen', ctypes.c_ulong),
        ('Name', ctypes.c_ubyte * 1)  # ubyte is used instead of char to prevent auto-conversion to string of the wrong size.
    ]


# A callback function type for the SymSearch function.
PSYM_ENUMERATESYMBOLS_CALLBACK  = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.POINTER(SYMBOL_INFO), ctypes.c_ulong, ctypes.py_object)
# PSYM_ENUMERATESYMBOLS_CALLBACK  = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.POINTER(SYMBOL_INFO), ctypes.c_ulong, ctypes.c_void_p)

class SymSearchOpt:
    """
    This enum contains options for the SymSearch function
    """
    RECURSE = 0x2
    GLOBALS_ONLY = 0x4
    ALL_ITEMS = 0x8

class SymTagEnum:
    """
    This enum contains various SymTag values, that can be used as tag values for the SymSearch function.
    """
    Annotation = 8


def SymSearch(process_handle, base_address, index, tag, mask, address, enum_symbol_callback, user_context, options):
    c_callback = PSYM_ENUMERATESYMBOLS_CALLBACK(enum_symbol_callback)
    base_address = ctypes.c_ulonglong(base_address)
    address = ctypes.c_ulonglong(address)
    if not _SymSearch(process_handle, base_address, index, tag, mask, address, c_callback, ctypes.py_object(user_context), options):
        raise SearchError()


def SymLoadModuleEx(process_handle, file_handle, image_name, module_name, dll_base, dll_size, data, flags):
    dll_base = ctypes.c_ulonglong(dll_base)
    base_address = _SymLoadModuleEx(process_handle, file_handle, image_name, module_name, dll_base, dll_size, data, flags)
    if not base_address:
        raise LoadModuleError()
    return base_address


def SymUnloadModule64(process_handle, base_address):
    base_address = ctypes.c_ulonglong(base_address)
    if not _SymUnloadModule64(process_handle, base_address):
        raise UnLoadModuleError()
