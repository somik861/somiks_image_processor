from argparse import ArgumentParser
from pathlib import Path
import json
from _check_config import check_algo_config

_ARG_FILE: Path = Path()


def main() -> None:
    config = json.load(open(_ARG_FILE))
    check_algo_config(config)


if __name__ == '__main__':
    parser = ArgumentParser()

    parser.add_argument(
        'file',
        metavar='JSON',
        type=Path,
        help='File to examine',
    )

    args = parser.parse_args()

    _ARG_FILE = args.file

    main()
