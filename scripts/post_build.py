import os
import shutil
from pathlib import Path
from glob import glob
from shutil import ignore_patterns

from .utils import cli, log


@cli.command(short_help='Package up completed binaries')
def post_build():
    if not os.path.isdir('./build'):
        log('Build directory not present. Run the build first.')
        return

    if os.path.isdir('./dist'):
        shutil.rmtree('./dist')

    log('Copying post build files to dist')
    Path('./dist/DATA/INTERFACE/RML').mkdir(parents=True)
    shutil.copy2('./vendor/freetype.dll', 'dist/')

    # Dll Files
    result = [y for x in os.walk('./build') for y in glob(os.path.join(x[0], '*.dll'))]
    for dll in result:
        shutil.copy2(dll, './dist/')
        log(f'copied {dll}')

    # Lib Files
    result = [y for x in os.walk('./build') for y in glob(os.path.join(x[0], '*.lib'))]
    for lib in result:
        shutil.copy2(lib, './dist/')
        log(f'copied {lib}')

    # DLL files inside vendor folder
    result = [y for x in os.walk('./vendor') for y in glob(os.path.join(x[0], '*.dll'))]
    for dll in result:
        shutil.copy2(dll, './dist/')
        log(f'copied {dll}')

    # Include Files
    shutil.copytree('./vendor/RmlUi/Include/RmlUi', 'dist/RmlUi')
    shutil.copytree('./FLUF.UI/Include', 'dist/FLUF.UI')
    shutil.copytree('./FLUF/Include', 'dist/FLUF', ignore=ignore_patterns('Internal'))

    # RML/RCSS/Lua files, only run if examples are compiled
    if os.path.exists('./dist/group_info.dll'):
        shutil.copytree('./examples/group_info/Rml', 'dist/DATA/INTERFACE/RML', dirs_exist_ok=True)
        shutil.copytree('./examples/shared_assets', 'dist/DATA/INTERFACE/RML', dirs_exist_ok=True)

    log('Zipping up dist folder to build.zip')
    shutil.make_archive('build', 'zip', './dist')
