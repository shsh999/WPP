import os

from logger import logger


def write_tmf_trace(tmf_file, pdb_path, trace):
    """
    Write a single trace information into the given TMF file.
    """
    tmf_file.write('// PDB:  ' + os.path.basename(pdb_path) + '\n')
    # Real TMFs also contain a `// last updated time`
    # Write general info
    tmf_file.write('{} {} // SRC={} MJ= MN=\n'.format(trace.guid,
                                                    trace.dir_name, trace.file_name))
    tmf_file.write('#typev {} 10 {} //  LEVEL={} FLAGS={} FUNC={}\n'.format(
        trace.file_name.replace('.', '_') + str(trace.line),
        trace.get_legacy_wpp_format(),
        'TRACE_LEVEL_{}'.format(trace.level.upper()),
        'WPP_FLAG_{}'.format(trace.flag),
        trace.func.replace(' ', '-')
    ))
    
    # Write arguments
    tmf_file.write('{\n')
    for i, arg_type in enumerate(trace.legacy_wpp_arg_types):
        # `i + 10` is used as the argument id, as arguments 0..9 are reserved
        tmf_file.write('_, {} -- {}\n'.format(arg_type, i + 10))

    tmf_file.write('}\n')


def generate_tmf_file(output_directory, pdb_path, trace):
    """
    Generate a TMF file for a single trace.
    The TMF file name will be 'output_directory\\<guid>.tmf', where <guid> is the trace GUID.

    :param output_directory: The path to the directory in which the new TMF file will be created.
    :type output_directory: str
    :param pdb_path: The path to the PDB path from which the trace information was generated.
    :type pdb_path: str
    :param trace: The trace information to be written into the TMF file.
    :type trace: trace_info.TraceInfo
    """
    output_path = os.path.join(output_directory, '{}.tmf'.format(trace.guid))
    with logger.set_trace_context(trace):
        logger.info('Generating trace file: "%s"', output_path)
        with open(output_path, 'w') as tmf_file:
            write_tmf_trace(tmf_file, pdb_path, trace)


def generate_tmf_file_for_multiple_traces(output_path, pdb_path, traces):
    """
    Generate a single TMF file containing multiple traces data.

    :param output_path: The path to the new TMF file.
    :type output_path: str
    :param pdb_path: The path to the PDB path from which the trace information was generated.
    :type pdb_path: str
    :param traces: The iterable of trace information to be written into the TMF file.
    :type traces: iterable[trace_info.TraceInfo]
    """
    with open(output_path, 'w') as tmf_file:
        for trace in traces:
            with logger.set_trace_context(trace):
                write_tmf_trace(tmf_file, pdb_path, trace)
