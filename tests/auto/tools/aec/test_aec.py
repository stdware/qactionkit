"""Tests the manifests that the Action Extension Compiler (AEC) accepts and rejects.

    python3 test_aec.py <path to qak_aec> <directory of cases>

Each case is a manifest in the directory of cases. A comment in the manifest specifies the
expected result:

    <!-- expect: error <regular expression> -->
    <!-- expect: success -->

An error case requires AEC to exit with a nonzero status and to print a message matching the
regular expression to the standard error. A success case requires AEC to exit with status zero.
The contents of the generated source file are tested separately, by compiling manifests into
test programs.
"""
import os
import re
import subprocess
import sys
import tempfile

EXPECT = re.compile(r"<!--\s*expect:\s*(error|success)\s*(.*?)\s*-->", re.DOTALL)


def expectation(path):
    with open(path, encoding="utf-8") as f:
        match = EXPECT.search(f.read())
    if not match:
        raise ValueError(f"{os.path.basename(path)}: no expect comment")
    return match.group(1), match.group(2)


def run(aec, path, output):
    return subprocess.run([aec, path, "-o", output], capture_output=True, text=True,
                          encoding="utf-8", errors="replace", timeout=60)


def check(aec, path, output):
    """Returns None if the case passes, and the reason of the failure otherwise."""
    kind, pattern = expectation(path)
    result = run(aec, path, output)
    if kind == "success":
        if result.returncode != 0:
            return f"expected success, but AEC failed: {result.stderr.strip()}"
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
        output = os.path.join(temporary, "out.cpp")
        for name in cases:
            reason = check(aec, os.path.join(directory, name), output)
            if reason is None:
                print(f"PASS   {name}")
            else:
                print(f"FAIL   {name}: {reason}")
                failures += 1

    print(f"{len(cases) - failures} passed, {failures} failed")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
