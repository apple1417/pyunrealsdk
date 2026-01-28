from .parser import gathered_info, parse_file
from .preprocessor import Flavour

if __name__ == "__main__":
    from pprint import pprint

    from .preprocessor import PYUNREALSDK_SRC

    file = PYUNREALSDK_SRC / "pyunrealsdk" / "base_bindings.cpp"
    parse_file(file, Flavour.OAK2)

    pprint(gathered_info, width=160)
