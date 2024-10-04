import os
import shutil
from glob import glob
from _utils import cli, log


@cli.command(short_help='Package up completed binaries')
def post_build():
    if not os.path.isdir('build'):
        log('Build directory not present. Run the build first.')
        return

    if os.path.isdir('dist'):
        shutil.rmtree('dist')

    log('Copying post build files to dist')
    os.mkdir('dist')
    shutil.copy2('vendor/freetype.dll', 'dist/')

    result = [y for x in os.walk('build') for y in glob(os.path.join(x[0], '*.dll'))]
    for dll in result:
        shutil.copy2(dll, 'dist/')
        log(f'copied {dll}')

    shutil.copytree('vendor/RmlUi/Include/RmlUi', 'dist/RmlUi')
    shutil.copytree('FLUF.UI/Include', 'dist/FLUF.UI')

    log('Zipping up dist folder to build.zip')
    shutil.make_archive('build', 'zip', 'dist')
