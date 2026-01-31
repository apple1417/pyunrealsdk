from __future__ import annotations

import textwrap
from collections.abc import Iterator
from dataclasses import dataclass, field
from enum import Enum, auto

__all__: tuple[str, ...] = (
    "ArgInfo",
    "AttrInfo",
    "ClassInfo",
    "EnumInfo",
    "FuncInfo",
    "FuncType",
    "InfoDict",
    "ModuleInfo",
)


type InfoDict = dict[str, AttrInfo | ClassInfo | EnumInfo | FuncInfo | ModuleInfo]


def format_docstring(docstring: str | None, indent: str = "") -> str:
    """
    Formats a docstring.

    Args:
        docstring: The raw docstring.
        indent: How much to indent the docstring.
    Returns:
        The formatted docstring.
    """
    if docstring is None:
        return ""
    if "\n" not in docstring:
        return textwrap.indent(f'"""{docstring}"""', indent)
    return textwrap.indent(f'"""\n{docstring}"""', indent)


@dataclass
class ModuleInfo:
    name: str
    docstring: str | None = None

    def declare(self) -> str:
        return format_docstring(self.docstring)


@dataclass
class AttrInfo:
    name: str
    type_hint: str

    def declare(self) -> str:
        return f"{self.name}: {self.type_hint}"


@dataclass
class ArgInfo:
    name: str
    type_hint: str | None  # Should only be none on self/cls and the pos-only/kw-only markers
    default: str | None

    def declare(self) -> str:
        output = self.name
        if self.type_hint:
            output += f": {self.type_hint}"
        if self.default:
            output += f" = {self.default}"
        return output


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

    def declare(self) -> str:
        output = ""
        if self.func_type == FuncType.StaticMethod:
            output += "@staticmethod\n"
        elif self.func_type == FuncType.ClassMethod:
            output += "@classmethod\n"

        output += f"def {self.name}({', '.join(a.declare() for a in self.args)}) -> {self.ret}:\n"

        if self.docstring:
            output += format_docstring(self.docstring, "    ")
            output += "\n"
        else:
            output += "    ...\n"

        return output


@dataclass
class ClassInfo:
    name: str
    super_class: str | None
    docstring: str | None = None
    attrs: list[AttrInfo] = field(default_factory=list)
    methods: list[FuncInfo] = field(default_factory=list)

    def declare(self) -> str:
        output = f"class {self.name}"
        if self.super_class is not None:
            output += f"({self.super_class})"
        output += ":\n"

        if self.docstring:
            output += format_docstring(self.docstring, "    ")
            output += "\n"

        for attr in self.attrs:
            output += f"    {attr.declare()}\n"
        if self.attrs:
            output += "\n"

        if self.methods:
            for method in self.iter_methods():
                output += textwrap.indent(method.declare(), "    ")

        if not any((self.docstring, self.attrs, self.methods)):
            output += "    ...\n"

        return output

    def iter_methods(self) -> Iterator[FuncInfo]:
        init: FuncInfo | None = None
        new: FuncInfo | None = None
        magic_methods: list[FuncInfo] = []
        standard_methods: list[FuncInfo] = []
        for method in self.methods:
            name = method.name
            if name == "__init__":
                init = method
            elif name == "__new__":
                new = method
            elif name.startswith("__") and name.endswith("__"):
                magic_methods.append(method)
            else:
                standard_methods.append(method)

        if init:
            yield init
        if new:
            yield new
        yield from sorted(magic_methods, key=lambda m: m.name)
        yield from sorted(standard_methods, key=lambda m: m.name)


@dataclass
class EnumValueInfo:
    name: str
    docstring: str | None = None

    def declare(self) -> str:
        output = f"{self.name} = ..."
        if self.docstring:
            output += "\n"
            output += format_docstring(self.docstring)

        return output


@dataclass
class EnumInfo:
    name: str
    values: list[EnumValueInfo] = field(default_factory=list)
    docstring: str | None = None

    def declare(self) -> str:
        output = f"class {self.name}(Enum):\n"
        if self.docstring:
            output += format_docstring(self.docstring, "    ")
            output += "\n"

        output += "\n"
        for val in self.values:
            output += textwrap.indent(val.declare(), "    ") + "\n"

        return output
