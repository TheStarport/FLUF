import os
import shutil
from pathlib import Path
from glob import glob
from shutil import ignore_patterns

import click

from .utils import cli, log


@cli.command(short_help='Package up completed binaries')
@click.option("--dest", type=str, help="Copy to another destination when done")
@click.option("--release", is_flag=True, help="Include/Exclude files depending on release mode", default=False)
def post_build(release: bool, dest: str):
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
        if (release and 'Debug' in dll) or (not release and 'Release' in dll):
            continue
        shutil.copy2(dll, './dist/')
        log(f'copied {dll}')

    # Lib Files
    result = [y for x in os.walk('./build') for y in glob(os.path.join(x[0], '*.lib'))]
    for lib in result:
        if (release and 'Debug' in lib) or (not release and 'Release' in lib):
            continue
        shutil.copy2(lib, './dist/')
        log(f'copied {lib}')

    # DLL files inside vendor folder
    result = [y for x in os.walk('./vendor') for y in glob(os.path.join(x[0], '*.dll'))]
    for dll in result:
        if (release and 'Debug' in dll) or (not release and 'Release' in dll):
            continue
        shutil.copy2(dll, './dist/')
        log(f'copied {dll}')

    # Include Files
    shutil.copytree('./vendor/RmlUi/Include/RmlUi', 'dist/RmlUi')
    shutil.copytree('./FLUF.UI/Include', 'dist/FLUF.UI')
    shutil.copytree('./FLUF/Include', 'dist/FLUF', ignore=ignore_patterns('Internal'))
    shutil.copy2('./vendor/imgui-markdown/imgui_markdown.h', 'dist/imgui_markdown.h')

    shutil.copy2('./vendor/curl-ca-bundle.crt', './dist/')
    log(f'copied ./vendor/curl-ca-bundle.crt')

    # Fonts
    result = [y for x in os.walk('./fonts') for y in glob(os.path.join(x[0], '*.ttf'))]
    os.mkdir('./dist/DATA/FONTS/')
    for font in result:
        shutil.copy2(font, './dist/DATA/FONTS/')
        log(f'copied {font}')

    # RML/RCSS/Lua files, only run if examples are compiled
    if os.path.exists('./dist/group_info.dll'):
        shutil.copytree('./examples/group_info/Rml', 'dist/DATA/INTERFACE/RML', dirs_exist_ok=True)
        shutil.copytree('./examples/shared_assets', 'dist/DATA/INTERFACE/RML', dirs_exist_ok=True)

    log('Zipping up dist folder to build.zip')
    shutil.make_archive('build', 'zip', './dist')

    if dest:
        log('Copying to end dest')
        if not dest.endswith(os.path.sep):
            dest += os.path.sep

        shutil.copytree('./dist', dest, dirs_exist_ok=True)
        shutil.copytree(dest + 'DATA', f'{dest}..{os.path.sep}DATA{os.path.sep}', dirs_exist_ok=True)
        shutil.rmtree(dest + 'DATA', ignore_errors=True)
