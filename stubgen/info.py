from __future__ import annotations

import textwrap
from dataclasses import dataclass, field
from enum import Enum, auto

__all__: tuple[str, ...] = (
    "ArgInfo",
    "AttrInfo",
    "ClassInfo",
    "FuncInfo",
    "FuncType",
    "InfoDict",
    "ModuleInfo",
)


type InfoDict = dict[str, ModuleInfo | AttrInfo | FuncInfo | ClassInfo]


@dataclass
class ModuleInfo:
    name: str
    docstring: str | None = None

    def declare(self) -> str:
        if self.docstring:
            if "\n" in self.docstring:
                return f'"""\n{self.docstring}\n"""'
            return f'"""{self.docstring}"""'
        return ""


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
            if "\n" in self.docstring:
                output += '    """\n'
                output += textwrap.indent(self.docstring, "    ")
                output += '    """\n'
            else:
                output += f'"""{self.docstring}"""'
        else:
            output += "    ...\n"

        return output


@dataclass
class ClassInfo:
    name: str
    docstring: str | None = None
    # TODO

    def declare(self) -> str:
        return f"class {self.name}: pass"
