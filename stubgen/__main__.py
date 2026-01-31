import shutil
from pathlib import Path

from .parser import parse_file
from .preprocessor import Flavour
from .renderer import render_stubs

if __name__ == "__main__":
    from pprint import pprint

    from .preprocessor import PYUNREALSDK_SRC

    file = PYUNREALSDK_SRC / "pyunrealsdk" / "base_bindings.cpp"
    info = parse_file(file, Flavour.OAK2)

    # pprint(info, width=160)

    output_dir = Path(__file__).parent / ".out"
    shutil.rmtree(output_dir, ignore_errors=True)

    render_stubs(output_dir, info)
