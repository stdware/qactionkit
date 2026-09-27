r"""Tests the manifests that the Action Extension Compiler (AEC) accepts and rejects.

    python3 test_aec.py <path to qak_aec> <directory of cases>

Each case is a manifest in the directory of cases. A comment in the manifest specifies the
expected result:

    <!-- expect: error <regular expression> -->
    <!-- expect: success -->

An error case requires AEC to exit with a nonzero status and to print a message matching the
regular expression to the standard error. A success case requires AEC to exit with status zero.
The contents of the generated files are tested separately, by compiling manifests into test
programs.

A second comment optionally gives the command line options of the case, as name=value pairs
without the leading dashes, which an XML comment cannot contain:

    <!-- options: function=f namespace=a::b -->

Each pair becomes --name value. Without the comment, the options are those of DEFAULT_OPTIONS,
and an empty comment gives no options. The output file and the header are always written to a
temporary directory.

A success case may also check the generated header, which must then match the regular expression
of every header comment, searched in the whole file:

    <!-- header: namespace a::b \{ -->
"""
import os
import re
import shlex
import subprocess
import sys
import tempfile

EXPECT = re.compile(r"<!--\s*expect:\s*(error|success)\s*(.*?)\s*-->", re.DOTALL)
OPTIONS = re.compile(r"<!--\s*options:(.*?)-->", re.DOTALL)
HEADER = re.compile(r"<!--\s*header:\s*(.*?)\s*-->", re.DOTALL)
DEFAULT_OPTIONS = ["--function", "testExtension"]


def expectation(path):
    """Returns the expected kind of result, the regular expression of an error, the options, and
    the regular expressions of the header."""
    with open(path, encoding="utf-8") as f:
        text = f.read()
    match = EXPECT.search(text)
    if not match:
        raise ValueError(f"{os.path.basename(path)}: no expect comment")
    headers = HEADER.findall(text)
    options = OPTIONS.search(text)
    if not options:
        return match.group(1), match.group(2), DEFAULT_OPTIONS, headers
    arguments = []
    for pair in shlex.split(options.group(1)):
        name, _, value = pair.partition("=")
        arguments += [f"--{name}", value]
    return match.group(1), match.group(2), arguments, headers


def run(aec, path, options, output, header):
    command = [aec, *options, "-o", output, "--header", header, path]
    return subprocess.run(command, capture_output=True, text=True, encoding="utf-8",
                          errors="replace", timeout=60)


def check(aec, path, directory):
    """Returns None if the case passes, and the reason of the failure otherwise."""
    kind, pattern, options, headers = expectation(path)
    header = os.path.join(directory, "out.qak.h")
    result = run(aec, path, options, os.path.join(directory, "out.cpp"), header)
    if kind == "success":
        if result.returncode != 0:
            return f"expected success, but AEC failed: {result.stderr.strip()}"
        if headers:
            with open(header, encoding="utf-8") as f:
                content = f.read()
            for expression in headers:
                if not re.search(expression, content):
                    return f"the header does not match \"{expression}\":\n{content}"
        return None
    if result.returncode == 0:
        return f"expected an error matching \"{pattern}\", but AEC succeeded"
    if not re.search(pattern, result.stderr):
        return f"expected an error matching \"{pattern}\", but AEC printed: {result.stderr.strip()}"
    return None


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    aec, directory = sys.argv[1], sys.argv[2]
    cases = sorted(name for name in os.listdir(directory) if name.endswith(".xml"))
    if not cases:
        print(f"no cases in {directory}")
        return 1

    failures = 0
    with tempfile.TemporaryDirectory() as temporary:
        for name in cases:
            reason = check(aec, os.path.join(directory, name), temporary)
            if reason is None:
                print(f"PASS   {name}")
            else:
                print(f"FAIL   {name}: {reason}")
                failures += 1

    print(f"{len(cases) - failures} passed, {failures} failed")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
