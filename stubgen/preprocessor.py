from __future__ import annotations

import re
from enum import StrEnum
from io import StringIO
from pathlib import Path
from typing import TYPE_CHECKING, Any

# Being slightly overzealous with the pyright ignores so that this will work even if you don't have
# pcpp installed in the same enviroment as pyright
# Even with it installed, it's not type hinted, half of them still need to exist
from pcpp.preprocessor import (  # pyright: ignore[reportMissingImports, reportMissingTypeStubs]
    Action,  # pyright: ignore[reportUnknownVariableType]
    OutputDirective,  # pyright: ignore[reportUnknownVariableType]
    Preprocessor,  # pyright: ignore[reportUnknownVariableType]
)

__all__: tuple[str, ...] = (
    "Flavour",
    "parse_macros_from_file",
)

if TYPE_CHECKING:
    from collections.abc import Iterator, Sequence

    class LexToken:
        type: str
        value: str
        lineno: int
        lexpos: int

    class Macro:
        name: str
        value: Sequence[LexToken]
        arglist: Sequence[str] | None
        variadic: bool
        varag: str | None

    type ArgTokens = Sequence[LexToken]


PYUNREALSDK_SRC = Path(__file__).parent.parent / "src"
UNREALSDK_SRC = PYUNREALSDK_SRC.parent / "libs" / "unrealsdk" / "src"


class Flavour(StrEnum):
    WILLOW = "WILLOW"
    OAK = "OAK"
    OAK2 = "OAK2"


class CustomPreprocessor(Preprocessor):  # pyright: ignore[reportUntypedBaseClass]
    allowed_includes: list[str]
    seen_macros: list[tuple[Macro, Sequence[ArgTokens]]]

    def __init__(self) -> None:
        super().__init__()  # pyright: ignore[reportUnknownMemberType]

        # Need to set these to make everything passthrough
        self.passthru_includes = re.compile("")
        self.passthru_expr_has_include = True

        self.seen_macros = []
        self.allowed_includes = []

    # Then need to overwrite these 4 methods to make everything passthrough
    def on_include_not_found(self, *_args: Any, **_kwargs: Any) -> None:  # noqa: D102  # pyright: ignore[reportIncompatibleMethodOverride]
        raise OutputDirective(Action.IgnoreAndPassThrough)  # pyright: ignore[reportUnknownMemberType]

    def on_unknown_macro_in_defined_expr(self, *_args: Any, **_kwargs: Any) -> None:  # noqa: D102  # pyright: ignore[reportIncompatibleMethodOverride]
        return None

    def on_unknown_macro_in_expr(self, *_args: Any, **_kwargs: Any) -> None:  # noqa: D102  # pyright: ignore[reportIncompatibleMethodOverride]
        return None

    def on_unknown_macro_function_in_expr(self, *_args: Any, **_kwargs: Any) -> None:  # noqa: D102  # pyright: ignore[reportIncompatibleMethodOverride]:
        return None

    # Hijack the include system to ignore most of them
    def include(  # noqa: D102
        self,
        tokens: Sequence[LexToken],
        *_args: Any,
        **_kwargs: Any,
    ) -> Iterator[LexToken]:
        if tokens[0].value in self.allowed_includes:
            yield from super().include(tokens, *_args, **_kwargs)  # pyright: ignore[reportUnknownMemberType]
            return
        raise OutputDirective(Action.IgnoreAndPassThrough)  # pyright: ignore[reportUnknownMemberType]

    # Hijack macro expansion to record them as they pass
    def macro_expand_args(  # noqa: D102  # pyright: ignore[reportIncompatibleMethodOverride]:
        self,
        macro: Macro,
        args: Sequence[ArgTokens],
        *_args: Any,
        **_kwargs: Any,
    ) -> Sequence[LexToken]:
        self.seen_macros.append((macro, args))
        return super().macro_expand_args(macro, args, *_args, **_kwargs)  # pyright: ignore[reportUnknownMemberType, reportUnknownVariableType]


def parse_macros_from_file(
    path: Path,
    flavour: Flavour,
) -> Iterator[tuple[Macro, Sequence[ArgTokens]]]:
    """
    Parses all custom macros out of the given file.

    Args:
        path: The file path to parse.
        flavour: The unrealsdk flavour to parse the file as.
    Yields:
        A series of macro-arg pairs, for each encountered macro.
    """
    pproc = CustomPreprocessor()
    pproc.add_path(PYUNREALSDK_SRC)  # pyright: ignore[reportUnknownMemberType]
    pproc.add_path(UNREALSDK_SRC)  # pyright: ignore[reportUnknownMemberType]

    # If we have something switched based on a flavour macro, our way of grabbing macros will match
    # both sides of the #if. Since we only want the real one, we need to do two passes

    # Firstly parse just the flavour.h to get all it's defines
    pproc.allowed_includes.append('"unrealsdk/flavour.h"')
    pproc.define("false 0")  # pyright: ignore[reportUnknownMemberType]
    pproc.define("true 1")  # pyright: ignore[reportUnknownMemberType]
    pproc.define(f"UNREALSDK_FLAVOUR UNREALSDK_FLAVOUR_{flavour}")  # pyright: ignore[reportUnknownMemberType]
    pproc.parse('#include "unrealsdk/flavour.h"\n')  # pyright: ignore[reportUnknownMemberType]
    pproc.write(StringIO())  # pyright: ignore[reportUnknownMemberType]

    # Then parse over the file only expanding flavour
    pproc.allowed_includes.clear()
    with path.open() as file:
        pproc.parse(file)  # pyright: ignore[reportUnknownMemberType]
    first_pass = StringIO()
    pproc.write(first_pass)  # pyright: ignore[reportUnknownMemberType]

    # Second parse, go over the results we just got, this time only expanding stubgen
    pproc.seen_macros.clear()
    pproc.allowed_includes.append('"pyunrealsdk/stubgen.h"')
    first_pass.seek(0)
    first_pass.name = "dummy"  # need a filename
    pproc.parse(first_pass)  # pyright: ignore[reportUnknownMemberType]
    pproc.write(StringIO())  # pyright: ignore[reportUnknownMemberType]

    yield from pproc.seen_macros
