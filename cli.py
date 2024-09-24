import click
import io
import os
import platform
import requests
import shutil
import subprocess
from zipfile import ZipFile


def is_windows():
    return platform.system() == "Windows" or os.getenv("GITHUB_ACTIONS") is not None


def log(s: str):
    click.echo(">>> " + s)


def try_purge(directory: str, name: str, should_purge: bool):
    if os.path.isdir(directory):
        if should_purge:
            log(f"Found existing {name}, purging...")
            shutil.rmtree(directory)
        else:
            raise EnvironmentError("Running the requirements command when existing directories are present requires "
                                   "the --purge flag to be present.")


def run(cmd: str, no_log: bool = False, allow_error: bool = False) -> int:

    log("Running Command: " + cmd)
    proc = subprocess.Popen(cmd, stdout=(subprocess.DEVNULL if no_log else subprocess.PIPE),
                            stderr=subprocess.STDOUT,  shell=True, text=True)

    if no_log:
        proc.wait()
    else:
        while proc.poll() is None:
            line = proc.stdout.readline()

            # if it ends with a new line character, remove it, so we don't print two lines
            if line.endswith('\n'):
                if line.endswith('\r\n'):
                    line = line[:-2]
                else:
                    line = line[:-1]

            print(line)

        print(proc.stdout.read())
        if not allow_error and proc.returncode != 0:
            raise ValueError(f"Execution of {cmd} failed")

    return proc.returncode


@click.group()
def cli():
    pass


@cli.command(short_help='Install and build dependencies via conan')
@click.option("-r", "--release", is_flag=True, help="Configure in release mode")
def install(release: bool):
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
    ctx.invoke(install, release=release)
    run(f'cmake --preset="{preset}"')
    run(f"cmake --build build/{('' if is_windows() else ('Release' if release else 'Debug'))}", allow_error=True)


if __name__ == '__main__':
    cli()
