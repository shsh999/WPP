from logger import logger

class TraceItem(object):
    pass


class LegacyTraceItem(TraceItem):
    """
    The base class for all trace items supporting conversion to "legacy" tmf files.
    """
    @classmethod
    def get_legacy_format(cls, format_spec):
        """
        Get the correct format specification according to the c++ specification.
        """
        raise NotImplementedError('{} does not support legacy format spec "{}"!'.format(cls.__name__, format_spec))

    @classmethod
    def get_legacy_item_name(cls):
        """
        Get the name of the legacy wpp item type. For example: ItemLong.
        """
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


class FloatingPointItem(LegacyTraceItem):
    @classmethod
    def get_legacy_format(cls, format_spec):
        if not format_spec:
            format_spec = 'f'
        if format_spec.lower() not in ('a', 'e', 'f', 'g'):
            return super(FloatingPointItem, cls).get_legacy_format(format_spec)
        return format_spec


class FloatItem(FloatingPointItem):
    @classmethod
    def get_legacy_item_name(cls):
        return 'ItemFloat'


class DoubleItem(FloatingPointItem):
    @classmethod
    def get_legacy_item_name(cls):
        return 'ItemDouble'


class LongDoubleItem(FloatingPointItem):
    @classmethod
    def get_legacy_item_name(cls):
        return 'ItemLongDouble'



TRACE_ITEM_TYPES = {
    cls.__name__: cls
    for cls in (
        CharItem, WCharItem, StringItem, WStringItem,
        Int8Item, Int16Item, Int32Item, Int64Item,
        UInt8Item, UInt16Item, UInt32Item, UInt64Item,
        SizeTItem, PtrDiffItem,
        FloatItem, DoubleItem, LongDoubleItem,
        PointerItem, GuidItem, HexBufferItem, HexDumpItem
    )
}

def make_trace_item_from_name(name):
    actual_name = name.split('wpp::')[-1]
    return TRACE_ITEM_TYPES[actual_name]()