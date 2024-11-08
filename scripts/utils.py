import os
import platform
import subprocess
from sys import stdout

import click
import requests


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
