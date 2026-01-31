from __future__ import annotations

import ast
import sys
from pathlib import Path
from typing import TYPE_CHECKING

from .info import ArgInfo, AttrInfo, ClassInfo, FuncInfo, FuncType, InfoDict, ModuleInfo
from .preprocessor import Flavour, parse_macros_from_file

if TYPE_CHECKING:
    from collections.abc import Sequence

    from .preprocessor import ArgTokens, LexToken

__all__: tuple[str, ...] = ("parse_file",)


gathered_info: InfoDict = {}
context_stack: list[ModuleInfo | FuncInfo | ClassInfo] = []


def parse_string(tokens: Sequence[LexToken]) -> str:
    """
    Parses a series of tokens which are meant to be a single string.

    Args:
        tokens: The sequence of tokens to parse.
    Returns:
        The parsed string.
    """
    assert all(t.type in {"CPP_STRING", "CPP_WS"} for t in tokens), "expected a string"
    # Stupid smart idea: Python string literals are strictly a superset of C strings, so we can
    # parse a C string just by pretending it's a Python literal
    # Have to ignore whitespace in case of unexpected indents
    return ast.literal_eval("".join(t.value for t in tokens if t.type != "CPP_WS"))


def _create_module(full_name: str) -> None:
    module: ModuleInfo
    if (existing := gathered_info.get(full_name)) is not None:
        assert isinstance(existing, ModuleInfo), f"module has same name as existing {existing}"
        module = existing
    else:
        module = ModuleInfo(full_name)
        gathered_info[full_name] = module

    context_stack[:] = [module]


def parse_module(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_MODULE macro.

    Args:
        args: The macro's args.
    """
    assert len(args) == 1, "expected one arg"
    name = parse_string(args[0])
    _create_module(name)


def parse_submodule(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_SUBMODULE macro.

    Args:
        args: The macro's args.
    """
    assert len(args) == 2, "expected two args"  # noqa: PLR2004
    outer = parse_string(args[0])
    name = parse_string(args[1])
    _create_module(outer + "." + name)


def parse_docstring(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_DOCSTRING macro.

    Args:
        args: The macro's args.
    """
    assert len(args) == 1, "expected one arg"
    docstring = parse_string(args[0])

    if "\n" in docstring:
        assert docstring[-1] == "\n", "expected multiline docstring to end with a newline"

    assert isinstance(context_stack[-1], ModuleInfo | FuncInfo | ClassInfo), (
        "can only add docstrings to a module, function, or class"
    )
    context_stack[-1].docstring = docstring


def parse_attr(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_ATTR macro.

    Args:
        args: The macro's args.
    """
    assert len(args) == 2, "expected two args"  # noqa: PLR2004
    name = parse_string(args[0])
    type_hint = parse_string(args[1])

    while not isinstance(context_stack[-1], ModuleInfo | ClassInfo):
        context_stack.pop()

    full_name = ".".join(x.name for x in context_stack) + "." + name

    assert full_name not in gathered_info, f"got duplicate attribute {full_name}"
    gathered_info[full_name] = AttrInfo(name, type_hint)


def parse_func(args: Sequence[ArgTokens], func_type: FuncType) -> None:
    """
    Parses one of the varous function stubgen macros.

    Args:
        args: The macro's args.
        func_type: What type of function is being parsed.
    """
    assert len(args) == 2, "expected two args"  # noqa: PLR2004
    name = parse_string(args[0])
    ret = parse_string(args[1])

    full_name = ".".join(x.name for x in context_stack) + "." + name

    assert full_name not in gathered_info, f"got duplicate function {full_name}"
    func = FuncInfo(func_type, name, ret)
    gathered_info[full_name] = func
    context_stack.append(func)


def parse_arg(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_ARG macro.

    Args:
        args: The macro's args.
    """

    assert len(args) >= 3, "expected at least 3 args"  # noqa: PLR2004

    assert args[0][-1].type == "CPP_ID" and args[0][-1].value == "_a", (
        'expected first arg to be a pybind "arg"_a literal'
    )
    name = parse_string(args[0][:-1])

    type_hint = parse_string(args[1])
    default = parse_string(args[2]) if args[2] else None

    assert isinstance(context_stack[-1], FuncInfo), "tried to add an arg outside of a function"
    context_stack[-1].args.append(ArgInfo(name, type_hint, default))


def parse_file(path: Path, flavour: Flavour) -> InfoDict:  # noqa: C901
    """
    Parses all data out of the given file.

    Args:
        path: The file to parse.
        flavour: What SDK flavour to parse using.
    Returns:
        The parsed info.
    """
    gathered_info.clear()
    context_stack.clear()

    for macro, args in parse_macros_from_file(path, flavour):
        try:
            match macro.name.removesuffix("_N"):
                case "PYUNREALSDK_STUBGEN_MODULE":
                    parse_module(args)
                case "PYUNREALSDK_STUBGEN_SUBMODULE":
                    parse_submodule(args)
                case "PYUNREALSDK_STUBGEN_DOCSTRING":
                    parse_docstring(args)
                case "PYUNREALSDK_STUBGEN_ATTR":
                    parse_attr(args)
                case "PYUNREALSDK_STUBGEN_FUNC":
                    while not isinstance(context_stack[-1], ModuleInfo):
                        context_stack.pop()
                    parse_func(args, FuncType.Func)
                case "PYUNREALSDK_STUBGEN_ARG":
                    parse_arg(args)
                case "PYUNREALSDK_STUBGEN_POS_ONLY" | "PYUNREALSDK_STUBGEN_KW_ONLY":
                    assert len(args) == 1 and not args[0], "expected no args"
                    assert isinstance(context_stack[-1], FuncInfo), (
                        "tried to add an arg marker outside of a function"
                    )
                    context_stack[-1].args.append(
                        ArgInfo(
                            "/" if macro.name == "PYUNREALSDK_STUBGEN_POS_ONLY" else "*",
                            None,
                            None,
                        ),
                    )
                case _:
                    assert not macro.name.startswith("PYUNREALSDK_STUBGEN"), (
                        "encountered unknown stubgen macro"
                    )
        except Exception as ex:
            try:
                path_str = str(path.relative_to(Path(__file__).parent.parent))
            except ValueError:
                path_str = str(path)
            sys.stderr.write(f"Failed to parse macro from {path_str}:\n{macro} {args}\n")
            raise ex

    return gathered_info
