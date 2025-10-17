import os
import shutil
from pathlib import Path
from glob import glob
from shutil import ignore_patterns
import traceback

import click

from .utils import cli, log


def _copy_all_files_of_type(src: str, dst: str, extensions: list[str], in_build: bool, release: bool):
    if in_build:
        src = os.path.join('build', 'RelWithDebInfo' if release else 'Debug', src)

    files = [
        [item for lst in [glob(os.path.join(src, '**', '*.' + e), recursive=True) for e in extensions] for item in
         lst]][0]
    for file in files:
        destination = './dist/' + dst
        os.makedirs(destination, exist_ok=True)
        shutil.copy2(file, destination)
        log(f'copied {file} to {destination}')


@cli.command(short_help='Package up completed binaries')
@click.option("--dest", type=str, help="Copy to another destination when done")
@click.option("--release", is_flag=True, help="Include/Exclude files depending on release mode", default=False)
def post_build(release: bool, dest: str):
    # noinspection PyBroadException
    try:
        if not os.path.isdir('./build'):
            log('Build directory not present. Run the build first.')
            return

        if os.path.isdir('./dist'):
            shutil.rmtree('./dist')

        build_path = os.path.join('build', 'RelWithDebInfo' if release else 'Debug')

        # Binary Files
        _copy_all_files_of_type(src='modules', dst='EXE/modules', extensions=['dll'], in_build=True, release=release)
        _copy_all_files_of_type(src='modules', dst='lib', extensions=['pdb', 'lib'], in_build=True, release=release)
        _copy_all_files_of_type(src='vendor', dst='EXE/modules', extensions=['dll'], in_build=False, release=release)
        shutil.copy2(build_path + '/FLUF/FLUF.dll', 'dist/EXE/')
        shutil.copy2(build_path + '/FLUF/FLUF.pdb', 'dist/lib/')

        # Include Files
        os.makedirs('./dist/include', exist_ok=True)
        shutil.copytree('./FLUF/Include', 'dist/include/FLUF', ignore=ignore_patterns('Internal'))

        # Module Includes
        shutil.copytree('./vendor/imgui', 'dist/include/imgui')
        shutil.copytree('./modules/FLUF.UI/Include', 'dist/include/FLUF.UI', ignore=ignore_patterns('Internal'))
        shutil.copy2('./vendor/imgui-markdown/imgui_markdown.h', 'dist/include/imgui_markdown.h')
        shutil.copy2('./vendor/ImGuiColorTextEdit/TextEditor.h', 'dist/include/TextEditor.h')

        # Assets
        log('Copying assets to dist')
        os.makedirs('./dist/DATA', exist_ok=True)
        shutil.copytree('./assets', 'dist', dirs_exist_ok=True)


        if dest:
            log('Copying to end dest')
            if not dest.endswith(os.path.sep):
                dest += os.path.sep

            module_path = f'{dest}{os.path.sep}modules'
            os.makedirs(module_path, exist_ok=True)
            shutil.copytree('./dist/EXE/modules', module_path, dirs_exist_ok=True)
            shutil.copytree('./dist/DATA', f'{dest}..{os.path.sep}DATA{os.path.sep}', dirs_exist_ok=True)
            shutil.copy2(f'dist/EXE/FLUF.dll', dest)
            shutil.copy2(f'dist/EXE/curl-ca-bundle.crt', dest)

        # Create ZIP
        log('Zipping up dist folder to build.zip')
        shutil.make_archive('build', 'zip', './dist')

    except Exception:
        print(traceback.format_exc())
