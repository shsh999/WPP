import json
import os
import string

from logger import logger
from trace_items import LegacyTraceItem, make_trace_item_from_name


class TraceInfo(object):
    """
    This class represents a single trace information.
    """
    def __init__(self, guid, general_info, types_info):
        self.guid = guid
        self.file, self.line, func, flag, level, self.format, self.args = general_info
        self.func = func[len('FUNC='):]
        self.flag = int(flag[len('FLAG='):])
        self.level = level.split('TraceLevel::')[-1]
        self.arg_types = tuple(make_trace_item_from_name(t) for t in types_info)
    
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
        Get a legacy wpp-style format string for the given trace information. 
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

        # Use json dumps for string escaping
        return json.dumps(result)
    
    @property
    def legacy_wpp_arg_types(self):
        """
        Get the legacy WPP item name for all the arguments of the current trace.
        """
        return [arg.get_legacy_item_name() for arg in self.arg_types]
