from argparse import ArgumentParser
from pathlib import Path
from util import iterate_configs
from subprocess import run

ORIGINALS: Path = Path(__file__).parent/'originals'
RESULT: Path = Path(__file__).parent/'expected_results'


def generate_results(exe: Path, override: bool):
    for source, dest, algos, options in iterate_configs():
        if not override and (RESULT/dest).exists():
            continue

        exe_list = [exe, ORIGINALS/source, RESULT/dest]
        for algo in algos:
            exe_list.extend(['-a', algo])
        exe_list.extend(['--algo_opt_string', options])
        exe_list.append('--allow_override')
        exe_list = list(map(lambda x: str(x), exe_list))

        print(f'Generating {dest} ...')
        run(exe_list, check=True)


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('executable', type=Path)
    parser.add_argument('--override', action='store_true',
                        default=False, required=False)

    args = parser.parse_args()
    generate_results(args.executable, args.override)
