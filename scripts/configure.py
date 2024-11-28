import click
import os
import shutil
from .utils import cli, run, is_windows


@cli.command(short_help='Install and build dependencies via conan')
def configure():
    default_profile = run("conan profile path default", allow_error=True)
    if default_profile:
        run("conan profile detect")

    host = 'windows' if is_windows() else 'linux'
    run(f"conan install . -s build_type=Debug --build missing -pr:b=default -pr:h=./profiles/{host}")
    run(f"conan install . -s build_type=Release --build missing -pr:b=default -pr:h=./profiles/{host}")


# noinspection PyShadowingBuiltins
@cli.command(short_help='Runs a first-time build, downloading any needed dependencies, and generating preset files.')
@click.option("-r", "--release", is_flag=True, help="Build in release mode")
@click.option("--no-post-build", is_flag=True, help="Configure in release mode")
@click.option("--no-hooks", is_flag=True, help="Disable FLUF core hooks")
@click.pass_context
def build(ctx: click.Context, release: bool, no_post_build: bool, no_hooks: bool):
    preset = 'release' if release else 'debug'

    # noinspection PyTypeChecker
    ctx.invoke(configure)

    run(f'cmake --preset="{preset}" {'-DNO_POST_BUILD=TRUE' if no_post_build else ''} {'-DNO_HOOKS=TRUE' if no_hooks else ''}')
    run(f"cmake --build build/{preset.title()}", allow_error=True)
