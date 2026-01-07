from itertools import product
from pathlib import Path
import sys


def generate() -> list[str]:
    lines = ['#pragma once']
    for dim in range(2, 5):
        components = 'xyzw'[:dim]

        lines.append('')
        lines.append(f'#define SWIZZLE_D{dim} ' + '\\')
        lines.append(f'\tusing self = vector_base <T, {dim}>; ' + '\\')
        for l in range(1, 5):
            r = 'scalar <T>' if l == 1 else f'vector <T, {l}>'
            lines.append(f'\tusing C{l} = {r};' + '\\')
            for var in product(components, repeat=l):
                var = ''.join(var)
                code = 'e' + var.upper()
                lines.append(f'\t[[no_unique_address]] swizzle_component <SwizzleCode::{code}, self, C{l}> {var}; ' + '\\')
    return lines


def main() -> int:
    lines = generate()
    if len(sys.argv) > 1:
        Path(sys.argv[1]).write_text('\n'.join(lines) + '\n')
    else:
        print('\n'.join(lines))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
