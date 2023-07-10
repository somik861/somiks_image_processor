from os import replace
from pathlib import Path

format_configs = Path(__file__).parent.parent/'formats'/'configs'
algo_configs = Path(__file__).parent.parent/'algorithms'/'configs'
target_file = Path(__file__).parent.parent/'application' / \
    'managers'/'inlined_configs.hpp'

target_stream = open(target_file, mode='w', encoding='utf-8')


def _write_configs(var_name: str, directory: Path) -> None:
    target_stream.write(
        f'inline std::unordered_map<std::string, std::string> {var_name} = {{')

    configs = []
    for entry in directory.iterdir():
        content = open(
            entry, 'r', encoding='utf-8').read().replace('\n', ' ').replace('"', '\\"')
        while '  ' in content:
            content = content.replace('  ', ' ')
        configs.append(f'{{ "{entry.stem}", "{content}" }}')

    target_stream.write(',\n'.join(configs))
    target_stream.write('};\n')


target_stream.write(r'''
#pragma once
#include <string>
#include <unordered_map>

namespace ssimp::inlined_configs {
''')

_write_configs('formats', format_configs)
_write_configs('algorithms', algo_configs)

target_stream.write('}')
target_stream.close()
