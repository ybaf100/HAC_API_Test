#!/usr/bin/env python3
"""Validate the external API contract and compile consumer state tests."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
HEADERS = {
    "HAC.hpp": "dab3b113f5191eb3d55de91c6a456575f1987eb4",
    "Types.hpp": "2460dae6b486a27d0d2d5276f9841c23a90d047f",
}


def git_blob_sha(data):
    return hashlib.sha1(b"blob " + str(len(data)).encode() + b"\0" + data).hexdigest()


def validate_sources():
    mod = json.loads((ROOT / "mod.json").read_text())
    assert mod["id"] == "hwanhee1.hac-api-test"
    assert mod["id"] != "hwanhee1.hac"
    assert mod["dependencies"] == {
        "hwanhee1.hac": {"version": ">=v1.0.0", "required": True}
    }
    assert "api" not in mod, "Consumer must not masquerade as the provider"
    for name, expected in HEADERS.items():
        header = ROOT / "external" / "hwanhee1.hac" / "include" / name
        assert git_blob_sha(header.read_bytes()) == expected, f"Changed HAC ABI header: {name}"
    assert (ROOT / "external/hwanhee1.hac/LICENSE").is_file()
    cmake = (ROOT / "CMakeLists.txt").read_text()
    assert 'EXTERNALS "hwanhee1.hac:1.0.0"' in cmake
    assert 'SHARED src/main.cpp' in cmake
    source = (ROOT / "src/main.cpp").read_text()
    assert '#include <hwanhee1.hac/include/HAC.hpp>' in source
    assert 'return hac::api::getSnapshot();' in source
    assert source.count('hac::api::getSnapshot()') == 1
    assert 'std::chrono::milliseconds(100)' in source
    assert 'steady_clock::now()' in source
    assert 'scene->addChild(root, overlayZOrder)' in source
    assert 'WeakRef<CCScene> ownerScene' in source
    assert 'ownerScene.lock()' in source
    assert 'CCDirector::drawScene();' in source
    assert 'm_fields->' not in source, "CCDirector is not a CCNode"
    assert set(p.name for p in (ROOT / "src").glob('*.cpp')) == {'main.cpp'}
    assert 'HAC: CLEAN' not in (ROOT / "src/StatusPresentation.hpp").read_text()
    print("Passed dependency, public-header integrity, and consumer source checks", flush=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sanitize', action='store_true')
    args = parser.parse_args()
    validate_sources()
    compiler = shlex.split(os.environ.get('CXX', 'g++'))
    with tempfile.TemporaryDirectory(prefix='hac-consumer-test-') as temporary:
        binary = Path(temporary) / 'status-tests'
        flags = ['-std=c++23', '-Wall', '-Wextra', '-Werror', '-pedantic', '-g']
        if args.sanitize:
            flags += ['-fsanitize=address,undefined', '-fno-omit-frame-pointer', '-no-pie']
        subprocess.run(compiler + flags + [
            '-I', str(ROOT / 'external'), '-I', str(ROOT / 'src'),
            str(ROOT / 'tests/StatusPresentationTest.cpp'), '-o', str(binary)
        ], check=True)
        subprocess.run([str(binary)], check=True)


if __name__ == '__main__':
    main()
