import click
import io
import os
import platform
import requests
import shutil
import subprocess
from sys import stdout
from zipfile import ZipFile


def is_windows():
    return platform.system() == "Windows" or os.getenv("GITHUB_ACTIONS") is not None


def log(s: str):
    click.echo(">>> " + s)


def download_file(url: str, filename: str):
    with open(filename, "wb") as f:
        log("Downloading %s" % filename)
        response = requests.get(url, stream=True)
        total_length = response.headers.get('content-length')

        if total_length is None:  # no content length header
            f.write(response.content)
        else:
            dl = 0
            total_length = int(total_length)
            for data in response.iter_content(chunk_size=4096):
                dl += len(data)
                f.write(data)
                done = int(50 * dl / total_length)
                stdout.write("\r[%s%s]" % ('=' * done, ' ' * (50 - done)))
                stdout.flush()


def run(cmd: str, no_log: bool = False, allow_error: bool = False) -> int:
    log("Running Command: " + cmd)
    proc = subprocess.Popen(cmd, stdout=(subprocess.DEVNULL if no_log else subprocess.PIPE),
                            stderr=subprocess.STDOUT, shell=True, text=True)

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


@cli.command(short_help='Downloads and unpacks the DirectX9 SDK if needed')
def d3d9():
    if os.path.isdir("vendor/DXSDK"):
        log("DXSDK already exists. No need to download...")
        return

    download_file('https://download.microsoft.com/download/A/E/7/AE743F1F-632B-4809-87A9-AA1BB3458E31/DXSDK_Jun10.exe',
                  '_DX2010_.exe')

    log("Extracting contents from _DX2010_.exe ...")
    run("7z x _DX2010_.exe DXSDK/Include -ovendor", no_log=True)
    run("7z x _DX2010_.exe DXSDK/Lib/x86 -ovendor", no_log=True)

    log("Extracted contents successfully. Cleaning up...")
    os.remove("_DX2010_.exe")


@cli.command(short_help='Downloads and unpacks libcurl if needed')
def curl():
    if os.path.isdir("vendor/curl-8.10.1"):
        log("libcurl already exists. No need to download...")
        return

    download_file('https://curl.se/download/curl-8.10.1.tar.gz', 'libcurl.tar.gz')
    log("Extracting contents from libcurl.tar.gz ...")

    run('7z x "libcurl.tar.gz" -so | 7z x -aoa -si -ttar -ovendor', no_log=True)

    log("Extracted contents successfully. Cleaning up...")
    os.remove("libcurl.tar.gz")


@cli.command(short_help='Downloads and unpacks all (non-conan) dependencies')
@click.pass_context
def dependencies(ctx: click.Context):
    ctx.invoke(curl)
    ctx.invoke(d3d9)


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


if __name__ == '__main__':
    cli()
