import os
import shutil

import click
from .utils import cli, download_file, log, run, raise_if_missing_exe


@cli.command(short_help='Downloads and unpacks the DirectX9 SDK if needed')
def d3d8():
    raise_if_missing_exe('7z')

    if os.path.isdir("./vendor/DXSDK"):
        log("DXSDK already exists. No need to download...")
        return

    download_file('https://f003.backblazeb2.com/file/starport-attachments/dx8sdk.zip',
                  'dx8sdk.zip')

    log("Extracting contents from dx8sdk.zip ...")
    run("7z x dx8sdk.zip include -ovendor/DXSDK", no_log=True)
    run("7z x dx8sdk.zip lib -ovendor/DXSDK", no_log=True)

    log("Extracted DX8 successfully, patching bad header file...")
    # DX8 didn't ship with 64bit support which breaks it on modern compilers as it is missing a needed definition
    path = './vendor/DXSDK/include/basetsd.h'
    with open(path, 'r') as file:
        # read a list of lines into data
        lines = file.readlines()

    lines[39] = '#define POINTER_64 __ptr64\n'

    # and write everything back
    with open(path, 'w') as file:
        file.writelines(lines)

    log('Cleaning up...')
    os.remove("dx8sdk.zip")


@cli.command(short_help='Downloads and unpacks libcurl if needed')
def curl():
    raise_if_missing_exe('7z')

    if os.path.isdir("./vendor/curl-8.10.1"):
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
    ctx.invoke(d3d8)
