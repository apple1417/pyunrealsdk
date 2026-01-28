from __future__ import annotations

import ast
import sys
from dataclasses import dataclass, field
from enum import Enum, auto
from pathlib import Path
from typing import TYPE_CHECKING

from .preprocessor import Flavour, parse_macros_from_file

if TYPE_CHECKING:
    from collections.abc import Sequence

    from .preprocessor import ArgTokens, LexToken

__all__: tuple[str, ...] = (
    "ArgInfo",
    "AttrInfo",
    "ClassInfo",
    "FuncInfo",
    "FuncType",
    "ModuleInfo",
    "gathered_info",
    "parse_file",
)


gathered_info: dict[str, ModuleInfo | AttrInfo | FuncInfo | ClassInfo] = {}
context_stack: list[ModuleInfo | FuncInfo | ClassInfo] = []


@dataclass
class ModuleInfo:
    name: str
    docstring: str | None = None


@dataclass
class AttrInfo:
    name: str
    type_hint: str


@dataclass
class ArgInfo:
    name: str
    type_hint: str | None  # Should only be none on self/cls and the pos-only/kw-only markers
    default: str | None


class FuncType(Enum):
    Func = auto()
    Method = auto()
    StaticMethod = auto()
    ClassMethod = auto()


@dataclass
class FuncInfo:
    func_type: FuncType
    name: str
    ret: str
    args: list[ArgInfo] = field(default_factory=list)
    docstring: str | None = None


@dataclass
class ClassInfo:
    name: str
    docstring: str | None = None
    # TODO


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


def parse_module(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_MODULE macro.

    Args:
        args: The macro's args.
    """
    assert len(args) == 1, "expected one arg"
    name = parse_string(args[0])

    module: ModuleInfo
    if (existing := gathered_info.get(name)) is not None:
        assert isinstance(existing, ModuleInfo), f"module has same name as existing {existing}"
        module = existing
    else:
        module = ModuleInfo(name)
        gathered_info[name] = module

    context_stack[:] = [module]


def parse_docstring(args: Sequence[ArgTokens]) -> None:
    """
    Parses a PYUNREALSDK_STUBGEN_DOCSTRING macro.

    Args:
        args: The macro's args.
    """
    assert len(args) == 1, "expected one arg"
    docstring = parse_string(args[0])

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


def parse_file(path: Path, flavour: Flavour) -> None:  # noqa: C901
    """
    Parses all data out of the given file, and adds it to 'gathered_info'.

    Args:
        path: The file to parse.
        flavour: What SDK flavour to parse using.
    """
    for macro, args in parse_macros_from_file(path, flavour):
        try:
            match macro.name.removesuffix("_N"):
                case "PYUNREALSDK_STUBGEN_MODULE":
                    parse_module(args)
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
                    assert not args, "expected no args"
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
