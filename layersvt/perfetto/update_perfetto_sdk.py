#!/usr/bin/env python3
"""
Updates the Amalgamated Perfetto SDK files (perfetto.h and perfetto.cc)
by downloading the SDK zip from the official GitHub release.

Usage:
    python3 update_perfetto.py [version]

    Example:
    python3 update_perfetto_sdk.py v49.0
"""

import argparse
import io
import os
import re
import sys
import urllib.request
import zipfile

README_PATH = os.path.join(os.path.dirname(__file__), "README.md")
FILES_TO_EXTRACT = ["perfetto.h", "perfetto.cc"]

def main():
    # Ensure we write files to the script's directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)
    parser = argparse.ArgumentParser(description="Update Perfetto SDK files.")
    parser.add_argument("version", nargs="?", default=DEFAULT_VERSION, help="Perfetto version tag (e.g., v49.0)")
    args = parser.parse_args()

    version = args.version
    if not version.startswith("v"):
        version = f"v{version}"

    # User requested to use the source code zip (v49.0.zip)
    url = f"https://github.com/google/perfetto/archive/refs/tags/{version}.zip"

    print(f"Updating Perfetto SDK to {version}...")
    print(f"Attempting download from: {url}")

    found_zip_data = None

    try:
        with urllib.request.urlopen(url) as response:
            if response.status == 200:
                print("Download successful.")
                found_zip_data = response.read()
    except urllib.error.HTTPError as e:
        print(f"Failed: {e.code} {e.reason}")
        if e.code == 404:
            print(f"The zip file was not found at {url}. Please check the version.")
    except Exception as e:
        print(f"Error: {e}")

    if not found_zip_data:
        sys.exit(1)

    # Extract files
    try:
        with zipfile.ZipFile(io.BytesIO(found_zip_data)) as z:
            names = z.namelist()

            for target in FILES_TO_EXTRACT:
                # The SDK zip usually has files at root or in a folder like 'sdk/'
                # We search for the file ending with target
                candidates = [n for n in names if n.endswith(f"/{target}") or n == target]
                if not candidates:
                    print(f"Error: Could not find {target} in zip.")
                    sys.exit(1)

                # Prefer the one with shortest path (likely root or sdk/)
                candidate = sorted(candidates, key=len)[0]

                print(f"Extracting {candidate} -> {target}")
                with z.open(candidate) as src, open(target, "wb") as dst:
                    dst.write(src.read())

    except zipfile.BadZipFile:
        print("Error: Invalid zip file.")
        sys.exit(1)

    print("Files updated.")

    # Update README
    update_readme(version, url)

def update_readme(version, url):
    if not os.path.exists(README_PATH):
        return

    print("Updating README.md...")
    with open(README_PATH, "r") as f:
        content = f.read()

    # Replace Version
    content = re.sub(r"- \*\*Version\*\*: .*", f"- **Version**: {version}", content)
    # Replace Source
    content = re.sub(r"- \*\*Source\*\*: .*", f"- **Source**: {url}", content)

    with open(README_PATH, "w") as f:
        f.write(content)

if __name__ == "__main__":
    main()
