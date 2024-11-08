import os
import click
from .utils import cli, download_file, log, run


@cli.command(short_help='Downloads and unpacks the DirectX9 SDK if needed')
def d3d9():
    if os.path.isdir("./vendor/DXSDK"):
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
    # ctx.invoke(curl)
    ctx.invoke(d3d9)
