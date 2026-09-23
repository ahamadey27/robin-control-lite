#!/usr/bin/env python3
"""Run locally in Terminal to initialize WrapTool's own Keychain credentials."""

import getpass
from pathlib import Path
import subprocess
import sys


def main():
    tool = Path('/Applications/PACEAntiPiracy/Eden/Fusion/Current/bin/wraptool')
    if not sys.stdin.isatty():
        print('Run this script yourself in an interactive Terminal.', file=sys.stderr)
        return 1
    if not tool.is_file():
        print('PACE WrapTool is not installed at the expected location.', file=sys.stderr)
        return 1

    print('One-time PACE signing setup. Keep your developer iLok connected.')
    print('PACE will cache credentials in macOS Keychain and sync its local configuration cache.')
    print('No plugin is signed or installed by this script.')
    print('The password is hidden while typing and is not saved by this script.')
    print('WrapTool requires a password argument; it exists briefly in the child process arguments.')
    account = input('iLok User ID: ').strip()
    if not account:
        print('No account entered; cancelled.')
        return 1
    password = getpass.getpass('iLok password (hidden): ')
    if not password:
        print('No password entered; cancelled.')
        return 1
    try:
        result = subprocess.run(
            [str(tool), 'sync', '--account', account, '--password', password],
            stdin=subprocess.DEVNULL, capture_output=True, text=True,
            errors='replace', timeout=180,
        )
    except subprocess.TimeoutExpired:
        print('PACE setup timed out. No diagnostic output was printed.', file=sys.stderr)
        return 1
    except OSError:
        print('Could not launch PACE WrapTool.', file=sys.stderr)
        return 1
    # Never print the command or an exception containing its password argument.
    output = (result.stdout + result.stderr).replace(password, '[REDACTED]')
    if output.strip():
        print(output.strip())
    if result.returncode == 0:
        print('PACE synchronization completed. Return to Codex to continue AAX signing.')
    else:
        print('PACE setup failed. Share the error above, never your password.')
    return result.returncode


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (KeyboardInterrupt, EOFError):
        print('\nCancelled.')
        sys.exit(1)
