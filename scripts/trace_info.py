import json
import os
import string

from logger import logger

class TraceItem(object):
    pass


class LegacyTraceItem(TraceItem):
    @classmethod
    def get_legacy_format(cls, format_spec):
        raise NotImplementedError('{} does not support legacy format spec "{}"!'.format(cls.__name__, format_spec))

    @classmethod
    def get_legacy_item_name(cls):
        raise NotImplementedError()

class IntegralTraceItem(LegacyTraceItem):
    @classmethod
    def get_legacy_format(cls, format_spec):
        default_sign_specifier = 'd' if cls._IS_SIGNED else 'u'
        int_format_spec = format_spec or default_sign_specifier
        int_format_spec = int_format_spec.replace('d', default_sign_specifier)
        if 'b' in int_format_spec.lower():
            logger.warning('Got unsupported legacy format {}, replacing with hex version'.format(int_format_spec))
            int_format_spec = int_format_spec.replace('b', 'x').replace('B', 'X')

        if int_format_spec not in ('d', 'u', 'x', 'X', 'o'):
            return super(IntegralTraceItem, cls).get_legacy_format(format_spec)

        return cls._get_size_prefix() + int_format_spec

    @classmethod
    def _get_size_prefix(cls):
        size = cls._BYTE_SIZE
        if size == 8:
            return 'I64'
        if size == 4:
            return ''
        if size == 2:
            return 'h'
        if size == 1:
            return 'hh'
        if size == '?':  # architecture dependant
            return 'I'
        raise ValueError('Bad integer type size!')
    
    @classmethod
    def get_legacy_item_name(cls):
        return cls._LEGACY_ITEM_NAME


class SignedIntegralTraceItem(IntegralTraceItem):
    _IS_SIGNED = True


class UnsignedIntegralTraceItem(IntegralTraceItem):
    _IS_SIGNED = False


class Int8Item(SignedIntegralTraceItem):
    _BYTE_SIZE = 1
    _LEGACY_ITEM_NAME = 'ItemChar'
    
    @classmethod
    def get_legacy_format(cls, format_spec):
        if format_spec in ('', 'd'):
            logger.warning('Cannot print int8_t as signed, using the unsigned form!')
        
        return super(Int8Item, cls).get_legacy_format(format_spec)


class Int16Item(SignedIntegralTraceItem):
    _BYTE_SIZE = 2
    _LEGACY_ITEM_NAME = 'ItemShort'

class Int32Item(SignedIntegralTraceItem):
    _BYTE_SIZE = 4
    _LEGACY_ITEM_NAME = 'ItemLong'

class Int64Item(SignedIntegralTraceItem):
    _BYTE_SIZE = 8
    _LEGACY_ITEM_NAME = 'ItemLongLong'

class UInt8Item(UnsignedIntegralTraceItem):
    _BYTE_SIZE = 1
    _LEGACY_ITEM_NAME = 'ItemUChar'

class UInt16Item(UnsignedIntegralTraceItem):
    _BYTE_SIZE = 2
    _LEGACY_ITEM_NAME = 'ItemShort'

class UInt32Item(UnsignedIntegralTraceItem):
    _BYTE_SIZE = 4
    _LEGACY_ITEM_NAME = 'ItemLong'

class UInt64Item(UnsignedIntegralTraceItem):
    _BYTE_SIZE = 8
    _LEGACY_ITEM_NAME = 'ItemULongLong'

class SizeTItem(UnsignedIntegralTraceItem):
    _BYTE_SIZE = '?'
    _LEGACY_ITEM_NAME = 'ItemPtr'

    @classmethod
    def get_legacy_format(cls, format_spec):
        if format_spec.startswith('z'):
            format_spec = format_spec[1:]
        return super(SizeTItem, cls).get_legacy_format(format_spec)

class PtrDiffItem(SignedIntegralTraceItem):
    _BYTE_SIZE = '?'
    _LEGACY_ITEM_NAME = 'ItemPtr'

    @classmethod
    def get_legacy_format(cls, format_spec):
        if format_spec.startswith('z'):
            format_spec = format_spec[1:]
        return super(PtrDiffItem, cls).get_legacy_format(format_spec)

class CharItem(SignedIntegralTraceItem):
    _BYTE_SIZE = 1
    _LEGACY_ITEM_NAME = 'ItemChar'
    
    @classmethod
    def get_legacy_format(cls, format_spec):
        if format_spec in ('c', ''):
            return 'c'
        return super(CharItem, cls).get_legacy_format(format_spec)

class WCharItem(SignedIntegralTraceItem):
    _BYTE_SIZE = 2
    _LEGACY_ITEM_NAME = 'ItemShort'
    
    @classmethod
    def get_legacy_format(cls, format_spec):
        if format_spec in ('c', ''):
            return 'c'
        return super(WCharItem, cls).get_legacy_format(format_spec)

class PointerItem(LegacyTraceItem):
    @classmethod
    def get_legacy_format(cls, format_spec):
        if format_spec in ('', 'p'):
            return 'p'
        return super(PointerItem, cls).get_legacy_format(format_spec)
    
    @classmethod
    def get_legacy_item_name(cls):
        return 'ItemPtr'

class StringItem(LegacyTraceItem):
    @classmethod
    def get_legacy_format(cls, format_spec):
        if format_spec in ('', 's'):
            return 's'
        return super(StringItem, cls).get_legacy_format(format_spec)
    
    @classmethod
    def get_legacy_item_name(cls):
        return 'ItemString'

class WStringItem(LegacyTraceItem):
    @classmethod
    def get_legacy_format(cls, format_spec):
        if format_spec in ('', 's'):
            return 's'
        return super(WStringItem, cls).get_legacy_format(format_spec)
    
    @classmethod
    def get_legacy_item_name(cls):
        return 'ItemWString'

class GuidItem(LegacyTraceItem):
    @classmethod
    def get_legacy_format(cls, format_spec):
        if format_spec == '':
            return 'GUID'
        return super(GuidItem, cls).get_legacy_format(format_spec)
    
    @classmethod
    def get_legacy_item_name(cls):
        return 'ItemGuid'

class HexBufferItem(LegacyTraceItem):
    @classmethod
    def get_legacy_format(cls, format_spec):
        return 's'
    
    @classmethod
    def get_legacy_item_name(cls):
        return 'ItemHEXBytes'

class HexDumpItem(LegacyTraceItem):
    @classmethod
    def get_legacy_format(cls, format_spec):
        return 's'
    
    @classmethod
    def get_legacy_item_name(cls):
        return 'ItemHEXDump'


TRACE_ITEM_TYPES = {
    cls.__name__: cls
    for cls in (
        CharItem, WCharItem, StringItem, WStringItem,
        Int8Item, Int16Item, Int32Item, Int64Item,
        UInt8Item, UInt16Item, UInt32Item, UInt64Item,
        SizeTItem, PtrDiffItem,
        PointerItem, GuidItem, HexBufferItem, HexDumpItem
    )
}


class TraceInfo(object):
    """
    This class represents a single wpp-ng trace information.
    """
    def __init__(self, guid, general_info, types_info):
        self.guid = guid
        self.file, self.line, func, flag, level, self.format, self.args = general_info
        self.func = func[len('FUNC='):]
        self.flag = int(flag[len('FLAG='):])
        self.level = level.split('TraceLevel::')[-1]
        self.arg_types = tuple(TRACE_ITEM_TYPES[t.split('wpp::')[-1]]() for t in types_info)
    
    @property
    def file_name(self):
        return os.path.basename(self.file)
    
    @property
    def dir_name(self):
        return os.path.dirname(self.file)
    
    @property
    def supports_legacy_format(self):
        return all(isinstance(arg, LegacyTraceItem) for arg in self.arg_types)
    
    def get_legacy_wpp_format(self):
        """
        Get a wpp-style format string for the given wpp-ng trace information. 
        """
        # Escape % characters before starting to generate the new format
        escaped = self.format.replace('%', '%%')
        result = '%0 '
        parts = string.Formatter().parse(escaped)
        i = 0
        for part in parts:
            literal_text, field_name, format_spec, conversion = part
            assert not field_name, 'Found unexpected field name in format'
            assert not conversion, 'Found unexpected conversion in format'
            result += literal_text
            if format_spec is not None:
                result += '%{}!{}!'.format(i + 10, self.arg_types[i].get_legacy_format(format_spec))
                i += 1
        
        assert i == len(self.arg_types), 'Missing format specifiers!'

        return json.dumps(result)
    
    @property
    def legacy_wpp_arg_types(self):
        return [arg.get_legacy_item_name() for arg in self.arg_types]
