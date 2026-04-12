#!/usr/bin/env python3

# Adapted from https://github.com/brevzin/ctp/blob/382507e4848555d1950ee05a4c330b1266e8c771/make_single_header
#
# License below:
#
# MIT License
#
# Copyright (c) 2025 Barry Revzin
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import re

from pathlib import Path

class IncludeExpander:
    INCLUDE_PATTERN = re.compile(r"#\s*include\s*<([^>]+)>")

    PRAGMA_ONCE_PATTERN = re.compile(r"#\s*pragma\s+once")

    def __init__(self, *, include_dir):
        self.include_dir = Path(include_dir)

    def expand(self, output, input_path, *, visited=None):
        if visited is None:
            visited = set()

        input_path = Path(input_path).absolute()

        if input_path in visited:
            return

        visited.add(input_path)

        with input_path.open() as input:
            for line in input:
                stripped = line.strip()

                if self.PRAGMA_ONCE_PATTERN.fullmatch(stripped):
                    continue

                match = self.INCLUDE_PATTERN.fullmatch(stripped)

                if match:
                    include_path = self.include_dir / match.group(1)

                    if include_path.exists():
                        self.expand(output, include_path, visited=visited)

                        continue

                output.write(line)

if __name__ == "__main__":
    expander = IncludeExpander(include_dir="include")

    with open("cvl_single_header.hpp", "w") as f:
        # NOTE: The experimental header includes the rest of the library too.
        expander.expand(f, "include/cvl/cvl.hpp")
