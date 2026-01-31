import subprocess
from pathlib import Path

# Being slightly overzealous with the pyright ignores so that this will work even if you don't have
# jinja installed in the same enviroment as pyright
from jinja2 import (  # pyright: ignore[reportMissingImports]
    Environment,  # pyright: ignore[reportUnknownVariableType]
    FileSystemLoader,  # pyright: ignore[reportUnknownVariableType]
    select_autoescape,  # pyright: ignore[reportUnknownVariableType]
)

from .info import InfoDict

__all__: tuple[str, ...] = ("render_stubs",)


def render_stubs(output_dir: Path, info: InfoDict) -> None:
    """
    Renders all stub files.

    Args:
        output_dir: The dir to write the stubs into.
        info: The collected stub info.
    """
    loader = FileSystemLoader(Path(__file__).parent / "templates")  # pyright: ignore[reportUnknownVariableType]
    env = Environment(loader=loader, autoescape=select_autoescape())  # pyright: ignore[reportUnknownVariableType]

    def declare(value: str) -> str:
        return info.pop(value).declare()

    env.filters["declare"] = declare  # pyright: ignore[reportUnknownMemberType]

    for name in loader.list_templates():  # pyright: ignore[reportUnknownMemberType, reportUnknownVariableType]
        template = env.get_template(name)  # pyright: ignore[reportUnknownMemberType, reportUnknownVariableType]
        output = (output_dir / name).with_suffix("")  # pyright: ignore[reportUnknownMemberType, reportUnknownVariableType]
        output.parent.mkdir(parents=True, exist_ok=True)  # pyright: ignore[reportUnknownMemberType]
        output.write_text(template.render())  # pyright: ignore[reportUnknownMemberType]

    subprocess.run(["ruff", "format", output_dir], check=True)
    subprocess.run(["ruff", "check", "--fix", output_dir], check=True)
    subprocess.run(["ruff", "format", output_dir], check=True)
