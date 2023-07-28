from pathlib import Path
from typing import Generator, Any
import json

JSON = Any
CONFIG: Path = Path(__file__).parent/'algo_test_config.json'


def combinations(dct: JSON) -> Generator[JSON, None, None]:
    flatten = {}
    for algo, options in dct.items():
        for opt, values in options.items():
            flatten[f'{algo}${opt}'] = values if type(
                values) is list else [values]

    flatten_idx_map = dict(
        map(lambda x: (x[1], x[0]), enumerate(flatten)))
    idx_flatted_map = dict(
        map(lambda x: (x[1], x[0]), flatten_idx_map.items()))
    possibilities = [0] * len(flatten)
    for opt, vals in flatten.items():
        possibilities[flatten_idx_map[opt]] = len(vals)

    # create all the options
    current_pos = [0] * len(possibilities)

    def increase_pos() -> None:
        nonlocal current_pos
        current_pos[0] += 1
        for i, (now, max_) in enumerate(zip(current_pos, possibilities)):
            if now < max_:
                break
            current_pos[i] = 0
            next_ = i + 1
            if next_ < len(current_pos):
                current_pos[next_] += 1

    def poss_to_json(poss: list[int]) -> JSON:
        out = {}
        for i, val_idx in enumerate(poss):
            option = idx_flatted_map[i]
            algo, opt = option.split('$')
            if not algo in out:
                out[algo] = {}

            value = flatten[option][val_idx]
            out[algo][opt] = value
        return out

    yield poss_to_json(current_pos)
    increase_pos()

    while any(map(lambda x: x != 0, current_pos)):
        yield poss_to_json(current_pos)
        increase_pos()


def name_from_opt(options: JSON) -> str:
    parts = []
    for algo, option_dct in options.items():
        parts.append(f'[{algo}]')
        for opt_name, opt_val in option_dct.items():
            if type(opt_val) is bool:
                opt_val = 'T' if opt_val else 'F'
            elif type(opt_val) is str:
                opt_val = opt_val if opt_val.isupper() else opt_val[:4]

            opt_name = opt_name[:5]

            parts.append(f'{opt_name}_{opt_val}')

    return '-'.join(parts)


# yields: rel. source, rel. dest, algos, options
def iterate_configs() -> Generator[tuple[Path, Path, list[str], str], None, None]:
    tests = json.load(open(CONFIG, 'r', encoding='utf-8'))

    for test in tests:
        def _get(key):
            return test[key] if type(test[key]) is list else [test[key]]

        sources = _get('sources')
        algos = _get('algos')

        for source in sources:
            for comb in combinations(test['options']):
                dest = Path(source + '_' + name_from_opt(comb) + '.png')
                yield source, dest, algos, json.dumps(comb)
