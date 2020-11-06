"""
This is the wpp-ng equivalent of tracepdb.exe, used to generate TMF files from PDBs of wpp-ng projets.
"""
import argparse
import os
import sys

from tmf_file import generate_tmf_file, generate_tmf_file_for_multiple_traces
from pdb_parser import extrace_trace_info

from logger import setup_logger, logger


def parse_arguments():
    parser = argparse.ArgumentParser(description='''This is the wpp-ng equivalent of tracepdb.exe, used to generate TMF files from PDBs of wpp-ng projects.''')
    parser.add_argument('file', type=str, help='The path to the pdb file containing formatting instructions.')
    
    # Either create a tmf file for each trace or a single tmf file contating all the traces.
    out_group = parser.add_mutually_exclusive_group(required=True)
    out_group.add_argument('-o', '--output-directory', type=str, help='The output directory path. Will be created if it does not yet exist.')
    out_group.add_argument('-of', '--output-file', type=str, help='Create a single tmf file containing all the traces with the given name.')
    
    parser.add_argument('-v', '--verbose', help='Display verbose output (use -vv for debug output).', action='count', default=0)
    return parser.parse_args()
    

def main():
    args = parse_arguments()
    pdb_path = args.file

    setup_logger(args.verbose)
    
    if not os.path.isfile(pdb_path):
        logger.error('Invalid PDB path: "{}"!'.format(pdb_path))
        return 1

    traces = extrace_trace_info(pdb_path)

    logger.info('Found {} traces!'.format(len(traces)))

    if args.output_directory is not None:
        output_directory = args.output_directory
        if not os.path.exists(output_directory):
            logger.debug('Creating output directory "%s"', output_directory)
            os.mkdir(output_directory)
        else:
            logger.debug('Output directory "%s" already exists!', output_directory)
    
        for trace in traces:
            generate_tmf_file(output_directory, pdb_path, trace)
    else:
        output_file = args.output_file
        logger.info('Generating single trace file: "%s"', output_file)
        generate_tmf_file_for_multiple_traces(output_file, pdb_path, traces)
    
    return 0

if __name__ == '__main__':
    exit(main())