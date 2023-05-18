from argparse import ArgumentParser
from pathlib import Path
import json
import sys
from typing import Any

_ARG_FILE: Path = Path()


def _error(*args: Any, **kwargs: Any) -> None:
    print('[ERROR]', *args, **kwargs, file=sys.stderr)


def _warning(*args: Any, **kwargs: Any) -> None:
    print('[WARNING]', *args, **kwargs, file=sys.stderr)


def _report_ignored(path: tuple[str, ...], *args: str) -> None:
    _warning(f'{_symbol_path(*(path + args))} not recognized and will be ignored')


def _symbol_path(*args: str) -> str:
    return '\'' + '-> ' + ' -> '.join(args) + '\''


def _matches_option(value: Any, options: list[Any], path: tuple[str, ...], *args: str) -> bool:
    if options.count(value) == 0:
        _error(f'{_symbol_path(*path, *args)} does not have supported option. Found {value}; expected one of {options}.')
        return False
    return True


def _is_valid_type(object: object, type_: type, path:  tuple[str, ...], *args: str) -> bool:
    if type(object) is not type_:
        _error(
            f'{_symbol_path(*path, *args)} is supposed to be {type_}, but is {type(object)}')
        return False
    return True


def _nonempty(object: list | str | dict | tuple, path: tuple[str, ...], *args, error: bool = True) -> bool:
    if not object:
        (_error if error else _warning)(
            f'{_symbol_path(*(path + args))} is empty')
        return False
    return True


def _contains(dct: dict[str, Any], key: str, path: tuple[str, ...], *args: str, type_: type | None = None) -> bool:
    new_path = (*path, *args, key)
    if key not in dct:
        _error(f'{_symbol_path(*new_path)} not found')
        return False

    if type_ is not None and not _is_valid_type(dct[key], type_, new_path):
        return False

    return True


def check_options(options: list[Any], path: tuple[str, ...]) -> None:
    for i, opt in enumerate(options):
        new_path = (*path, str(i))
        if not _is_valid_type(opt, dict, new_path):
            continue

        if _contains(opt, 'description', new_path, type_=str):
            _nonempty(opt['description'], new_path, 'description')

        unused = set(opt.keys() - {'description', 'type'})
        if _contains(opt, 'type', new_path, type_=str) and _matches_option(
                opt['type'], ['int', 'float', 'checkbox', 'text', 'choice', 'subsection'], new_path, 'type'):

            match opt['type']:
                case 'int':
                    unused -= {'range', 'default'}
                    if _contains(opt, 'range', new_path, type_=list):
                        range_ = opt['range']
                        if len(range_) != 2 or not _is_valid_type(range_[0], int, new_path, 'range', '0')\
                                or not _is_valid_type(range_[1], int, new_path, 'range', '1')\
                                or range_[0] > range_[1] or range_[0] < -2**31 or range_[1] > 2 ** 31 - 1:
                            _error(
                                f'{_symbol_path(*new_path, "range")} is in invalid format')
                        else:
                            if _contains(opt, 'default', new_path, type_=int) and 'range' in opt:
                                if not (opt['range'][0] <= opt['default'] <= opt['range'][1]):
                                    _error(
                                        f'{_symbol_path(*new_path, "default")} not in range')

                case 'float':
                    unused -= {'range', 'default'}
                    if _contains(opt, 'range', new_path, type_=list):
                        range_ = opt['range']
                        if len(range_) != 2 or not _is_valid_type(range_[0], float, new_path, 'range', '0')\
                                or not _is_valid_type(range_[1], float, new_path, 'range', '1')\
                                or range_[0] > range_[1]:
                            _error(
                                f'{_symbol_path(*new_path, "range")} is in invalid format')
                        else:
                            if _contains(opt, 'default', new_path, type_=float) and 'range' in opt:
                                if not (opt['range'][0] <= opt['default'] <= opt['range'][1]):
                                    _error(
                                        f'{_symbol_path(*new_path, "default")} not in range')

                case 'checkbox':
                    unused -= {'default'}
                    _contains(opt, 'default', new_path, type_=bool)

                case 'text':
                    unused -= {'range', 'default'}
                    if _contains(opt, 'range', new_path, type_=list):
                        range_ = opt['range']
                        if len(range_) != 2 or not _is_valid_type(range_[0], int, new_path, 'range', '0')\
                                or not _is_valid_type(range_[1], int, new_path, 'range', '1')\
                                or range_[0] > range_[1] or range_[0] < -2**31 or range_[1] > 2 ** 31 - 1:
                            _error(
                                f'{_symbol_path(*new_path, "range")} is in invalid format')
                        else:
                            if _contains(opt, 'default', new_path, type_=str) and 'range' in opt:
                                if not (opt['range'][0] <= len(opt['default']) <= opt['range'][1]):
                                    _error(
                                        f'{_symbol_path(*new_path, "default")} not in range')

                case 'choice':
                    unused -= {'values', 'default'}
                    if _contains(opt, 'values', new_path, type_=list):
                        for i, ch in enumerate(opt['values']):
                            _is_valid_type(ch, str, new_path, str(i))

                        if _contains(opt, 'default', new_path, type_=str):
                            _matches_option(
                                opt['default'], opt['values'], new_path)

                case 'subsection':
                    unused -= {'options', 'default'}
                    if _contains(opt, 'options', new_path, type_=list):
                        check_options(opt['options'], new_path + ('',))

                    _contains(opt, 'default', new_path, type_=bool)

        for key in unused:
            _report_ignored(new_path, key)


def check_extensions(extensions: list[Any], path: tuple[str, ...]) -> None:
    _nonempty(extensions, path, error=False)
    for i, ext in enumerate(extensions):
        new_path = (*path, str(i))
        if not _is_valid_type(ext, dict, new_path):
            continue

        if _contains(ext, 'suffix', new_path, type_=str):
            _nonempty(ext['suffix'], new_path, 'text')

        if 'type' in ext and _is_valid_type(ext['type'], str, new_path, 'type'):
            _matches_option(ext['type'], ['plain', 'regex'], new_path, 'type')

        for key in set(ext.keys()) - {'text', 'type'}:
            _report_ignored(new_path, key)


def check_root(config: dict[str, Any]) -> None:
    keys = set(config.keys())
    opt = 'options'
    if _contains(config, opt, tuple(), type_=list):
        check_options(config[opt], (opt,))

    ext = 'extensions'
    if _contains(config, ext, tuple(), type_=list):
        check_options(config[ext], (ext,))

    for key in keys - {opt, ext}:
        _report_ignored((key,))


def main() -> None:
    config = json.load(open(_ARG_FILE))
    check_root(config)


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
