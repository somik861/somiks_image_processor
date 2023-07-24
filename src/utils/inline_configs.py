from os import replace
from pathlib import Path
from itertools import chain

format_configs = Path(__file__).parent.parent/'formats'/'configs'
algo_configs = Path(__file__).parent.parent/'algorithms'/'configs'
target_config_file = Path(__file__).parent.parent/'application' / \
    'managers'/'inlined_configs.hpp'

main_license = Path(__file__).parent.parent.parent/'LICENSE.md'
licenses = Path(__file__).parent.parent/'licenses'
target_license_file = Path(__file__).parent.parent / \
    'application'/'managers'/'inlined_licenses.hpp'


def _write_configs(var_name: str, directory: Path, target_stream) -> None:
    target_stream.write(
        f'inline std::unordered_map<std::string, std::string> {var_name} = {{')

    configs = []
    for entry in directory.iterdir():
        if not entry.is_file() or not entry.name.endswith('.json'):
            continue
        content = open(
            entry, 'r', encoding='utf-8').read().replace('\n', ' ').replace('"', '\\"')
        while '  ' in content:
            content = content.replace('  ', ' ')
        configs.append(f'{{ "{entry.stem}", "{content}" }}')

    target_stream.write(',\n'.join(configs))
    target_stream.write('};\n')


def _split_by_size(string: str, size: int) -> list[str]:
    out = []
    while len(string) > size:
        out.append(string[:size])
        string = string[size:]
    if string:
        out.append(string)

    return out


def _write_licenses(var_name: str, directory: Path, target_stream) -> None:
    target_stream.write(
        f'inline std::unordered_map<std::string, std::string> {var_name} = {{')

    configs = []
    for entry in chain(directory.iterdir(), [main_license]):
        if not entry.is_file() or not (entry.name.endswith('.lic') or entry.name.endswith('.md')):
            continue
        content_parts = _split_by_size(
            open(entry, 'r', encoding='utf-8').read(), 10_000)
        content_parts = list(map(lambda x: x.replace(
            '\n', '\\n').replace('"', '\\"'), content_parts))
        license_name = 'ssimp' if entry.stem == 'LICENSE' else entry.stem

        _delim = '"s + "'
        configs.append(
            f'{{ "{license_name}", "{ _delim.join(content_parts) }"s }}')

    target_stream.write(',\n'.join(configs))
    target_stream.write('};\n')


# CONFIGS
target_stream = open(target_config_file, mode='w', encoding='utf-8')
target_stream.write(r'''
#pragma once
#include <string>
#include <unordered_map>

using namespace std::literals;

namespace ssimp::inlined_configs {
''')

_write_configs('formats', format_configs, target_stream)
_write_configs('algorithms', algo_configs, target_stream)

target_stream.write('}')
target_stream.close()

# LICENSES

target_stream = open(target_license_file, mode='w', encoding='utf-8')
target_stream.write(r'''
#pragma once
#include <string>
#include <unordered_map>

using namespace std::literals;

namespace ssimp::inlined_configs {
''')

_write_licenses('licenses', licenses, target_stream)

target_stream.write('}')
target_stream.close()
