from LDMX.Framework import ldmxcfg
from pathlib import Path

def from_file(source_file):
    if not isinstance(source_file, Path):
        source_file = Path(source_file)
    if not source_file.is_file():
        raise ValueError(f'{source_file} is not accessible.')

    src = source_file.resolve()
    # assume class name is name of file (no extension) if not provided
    class_name = src.stem
    # use class name for instance name if not provided
    instance_name = class_name

    lib = src.parent / f'lib{src.stem}.so'
    if not lib.is_file() or src.stat().st_mtime > lib.stat().st_mtime:
        print(
            f'Processor source file {src} is newer than its compiled library {lib}'
            ' (or library does not exist), recompiling...'
        )
        import subprocess
        subprocess.run([
            'g++', '-fPIC', '-shared', # construct a shared library for dynamic loading
            '-o', str(lib), str(src), # define output file and input source file
            '-lFramework', # link to Framework library (and the event dictionary)
            '-lDetDescr', # link to DetDescr library
            '-I/usr/local/include/root', # include ROOT's non-system headers
            '-I@CMAKE_INSTALL_PREFIX@/include', # include ldmx-sw headers (if non-system)
            '-L@CMAKE_INSTALL_PREFIX@/lib', # include ldmx-sw libs (if non-system)
            ], check=True)
        print(f'done compiling {src}')

    instance = ldmxcfg.Analyzer(instance_name, class_name, str(lib))
    return instance

p = ldmxcfg.Process('ana')
p.sequence = [ from_file('MAC2.cxx') ]
import sys
p.inputFiles = sys.argv[1:]
p.histogramFile = 'hist-no-filtering-narrow.root'
