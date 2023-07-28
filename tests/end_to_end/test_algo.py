from argparse import ArgumentParser
from pathlib import Path
from util import iterate_configs
from subprocess import run
from skimage import io
import numpy as np

ORIGINALS: Path = Path(__file__).parent/'originals'
RESULTS: Path = Path(__file__).parent/'expected_results'


def check_images(file_1: Path, file_2: Path, *,
                 diff_tolerance: float = 1e-5,
                 max_diff: float = 1e-5,
                 percent_required: float = 100.0) -> None:
    img_1 = io.imread(file_1)
    img_2 = io.imread(file_2)

    diff = np.abs(img_1.astype(np.float64) - img_2.astype(np.float64))
    assert np.all(diff < max_diff), f'{np.max(diff)} < {max_diff}'

    total = diff.size
    correct = np.sum(diff <= diff_tolerance)
    assert correct / total * 100 >= percent_required,\
        f'{correct / total * 100} >= percent_required'


def validate_results(exe: Path) -> None:
    tmp_file = Path('out.png')
    try:
        for source, dest, algos, options in iterate_configs():
            exe_list = [exe, ORIGINALS/source, tmp_file]
            for algo in algos:
                exe_list.extend(['-a', algo])
            exe_list.extend(['--algo_opt_string', options])
            exe_list.append('--allow_override')
            exe_list = list(map(lambda x: str(x), exe_list))

            print(f'Checking {dest} ...')
            run(exe_list, check=True)
            check_images(RESULTS/dest, 'out.png', diff_tolerance=1.1,
                         max_diff=50.0)
    finally:
        if tmp_file.exists():
            tmp_file.unlink()


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument('executable', type=Path)

    args = parser.parse_args()
    validate_results(args.executable)
