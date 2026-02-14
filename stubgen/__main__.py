import shutil
from pathlib import Path

from .info import InfoDict, ModuleInfo
from .parser import parse_file
from .preprocessor import PYUNREALSDK_SRC, Flavour, parse_flavour_macros
from .renderer import render_stubs


def merge_info(a: InfoDict, b: InfoDict) -> InfoDict:
    """
    Safely merges two info dicts.

    Args:
        a: The first dict to merge.
        b: The second dict to merge.
    Returns:
        The merged info dict.
    """
    duplicates = {
        key
        for key in (a.keys() & b.keys())
        # allow identical modules
        if not (isinstance(a_val := a[key], ModuleInfo) and a_val == b[key])
    }
    assert not duplicates, f"found duplicate definitions for: {duplicates}"

    return a | b


def generate(flavour: Flavour, output: Path) -> None:
    """
    Generates all stub files for the given flavour.

    Args:
        flavour: The flavour to generate for.
        output: The output path to write stubs into.
    """
    all_info: InfoDict = {}
    for dirpath, _dirnames, filenames in PYUNREALSDK_SRC.walk():
        for name in filenames:
            info = parse_file(dirpath / name, flavour)
            all_info = merge_info(all_info, info)

    flavour_macros = parse_flavour_macros(flavour)

    shutil.rmtree(output, ignore_errors=True)
    render_stubs(output, all_info, flavour_macros)


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "flavour",
        type=Flavour,
        choices=Flavour,
        help="Which SDK flavour to build stubs for",
    )
    parser.add_argument(
        "output",
        nargs="?",
        default=Path(__file__).parent / ".out",
        type=Path,
        help="Where to write the stubs to.",
    )
    args = parser.parse_args()

    generate(args.flavour, args.output)
