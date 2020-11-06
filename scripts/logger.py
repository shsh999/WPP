from contextlib import contextmanager

import logging
import sys


class Logger(logging.LoggerAdapter):
    """
    This is a logger adapter class supporting a trace info as a context - allowing to specify in which trace an error occurs.
    """
    LOGGER_NAME = 'wpp-ng-logger'
    def __init__(self, logger, extra={}):
        super(Logger, self).__init__(logging.getLogger(Logger.LOGGER_NAME), extra=extra)
        self.trace_context = None
    
    def process(self, msg, kwargs):
        if self.trace_context is not None:
            trace_info = '{} : {}'.format(self.trace_context.file, self.trace_context.line)
            return '%-25s | %s' % (trace_info, msg), kwargs
        else:
            return msg, kwargs
    
    @contextmanager
    def set_trace_context(self, trace_info):
        if self.trace_context is not None:
            raise Exception('WAT')
        self.trace_context = trace_info
        try:
            yield
        finally:
            self.trace_context = None
    

# This is the global logger object that should be used to log events.
logger = Logger(logging.getLogger('wpp-ng-logger'))

def setup_logger(verbose):
    """
    Setup the global logger according to the verbosity level passed in the command-line.
    This method should be called only once!
    """
    if verbose >= 2:
        logging_level = logging.DEBUG
    elif verbose == 1:
        logging_level = logging.INFO
    else:
        logging_level = logging.WARNING
    
    logger.logger.setLevel(logging_level)
    handler = logging.StreamHandler(sys.stdout)
    handler.setFormatter(logging.Formatter('%(levelname)-10s| %(message)s'))
    handler.setLevel(logging_level)
    logger.logger.addHandler(handler)

    if verbose > 2:
        logging.warning('Exceeded max verbosity, using -vv')
