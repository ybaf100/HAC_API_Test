#!/usr/bin/env python3
"""Check the final combined test mod, not the separate HAC provider."""
import json
from pathlib import Path
import sys
import zipfile

MOD_ID = 'hwanhee1.hac-api-test'
output = Path(sys.argv[1])
package = output / f'{MOD_ID}.geode'
assert package.is_file(), f'Missing test mod: {package}'
with zipfile.ZipFile(package) as archive:
    names = set(archive.namelist())
    mod = json.loads(archive.read('mod.json'))
    assert mod['id'] == MOD_ID
    assert mod['dependencies']['hwanhee1.hac'] == {
        'version': '>=v1.0.0', 'required': True
    }
    assert 'api' not in mod
    expected = {
        f'{MOD_ID}.dll', f'{MOD_ID}.dylib', f'{MOD_ID}.ios.dylib',
        f'{MOD_ID}.android32.so', f'{MOD_ID}.android64.so'
    }
    assert expected <= names, f'Missing platform binaries: {sorted(expected - names)}'
    assert not any(name.startswith('hwanhee1.hac.') for name in names), 'Embedded provider binary'
    assert not any(name.endswith('.hpp') for name in names), 'Build-only headers were packaged'
print(f'Validated required HAC dependency and all five platform binaries: {package.name}')
