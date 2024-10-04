import click
import os
import shutil
from _utils import cli, run, is_windows


@cli.command(short_help='Install and build dependencies via conan')
@click.option("-r", "--release", is_flag=True, help="Configure in release mode")
def configure(release: bool):
    default_profile = run("conan profile path default", allow_error=True)
    if default_profile:
        run("conan profile detect")

    presets = "CMakePresets.json"
    if os.path.exists(presets):
        os.remove(presets)

    if is_windows():
        shutil.copy2("CMakePresetsWindows.json", presets)
        run(f"conan install . --build missing -pr:b=default -pr:h=./profiles/windows-{('dbg' if release else 'rel')}")
        run(f"conan install . --build missing -pr:b=default -pr:h=./profiles/windows-{('rel' if release else 'dbg')}")
    else:
        shutil.copy2("CMakePresetsLinux.json", presets)
        run(f"conan install . --build missing -pr:b=default -pr:h=./profiles/linux-dbg")
        run(f"conan install . --build missing -pr:b=default -pr:h=./profiles/linux-rel")


@cli.command(short_help='Runs a first-time build, downloading any needed dependencies, and generating preset files.')
@click.option("-r", "--release", is_flag=True, help="Build in release mode")
@click.pass_context
def build(ctx: click.Context, release: bool):
    preset = 'build' if is_windows() else ('release' if release else 'debug')

    # noinspection PyTypeChecker
    ctx.invoke(configure, release=release)
    run(f'cmake --preset="{preset}"')
    run(f"cmake --build build/{('' if is_windows() else ('Release' if release else 'Debug'))}", allow_error=True)
