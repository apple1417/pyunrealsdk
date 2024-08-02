# ruff: noqa: D205, FIX004

from __future__ import annotations

from collections.abc import Callable
from enum import EnumMeta
from types import EllipsisType
from typing import Any, ClassVar, Literal, overload

from unrealsdk.unreal import BoundFunction, UObject, WrappedStruct

__all__: tuple[str, ...] = (
    "Block",
    "Type",
    "Unset",
    "add_hook",
    "has_hook",
    "inject_next_call",
    "log_all_calls",
    "remove_hook",
)

class Block:
    """
    A sentinel used to indicate a hook should block execution of the unrealscript
    function.
    """

# HACK: Pybind enums are completely normal classes, they don't inherit from the standard library
#       enums, which means we're not allowed to use them in Literal type hints.
#       If we pretend we use EnumMeta, suddenly we are.
#       While this is a blatant lie, for all normal usage it should still hint correctly, and it
#       lets us use overloads on add_hook()
class Type(metaclass=EnumMeta):
    """
    Enum of possible hook types.

    Members:

      PRE : Called before running the hooked function.

      POST : Called after the hooked function, but only if it was allowed to run.

      POST_UNCONDITIONAL : Called after the hooked function, even if it got blocked.
    """

    __members__: ClassVar[dict[str, Type]]
    __entries: ClassVar[dict[str, tuple[Type, str]]]

    PRE = ...
    POST = ...
    POST_UNCONDITIONAL = ...

    def __eq__(self, other: object) -> bool: ...
    def __getstate__(self) -> int: ...
    def __hash__(self) -> int: ...
    def __index__(self) -> int: ...
    def __init__(self, value: int) -> None: ...
    def __int__(self) -> int: ...
    def __ne__(self, other: object) -> bool: ...
    def __repr__(self) -> str: ...
    def __setstate__(self, state: int) -> None: ...
    @property
    def name(self) -> str: ...
    @property
    def value(self) -> int: ...

class Unset:
    """
    A sentinel used to indicate a return value override is unset - i.e. the actual
    return value will be used.
    """

type _HookBlockSignal = None | EllipsisType | Block | type[Block]
type _PreHookCallback = Callable[
    [UObject, WrappedStruct, Any, BoundFunction],
    _HookBlockSignal | tuple[_HookBlockSignal, Any],
]
type _PostHookCallback = Callable[[UObject, WrappedStruct, Any, BoundFunction], None]

type _PreHookType = Literal[Type.PRE]
type _PostHookType = Literal[Type.POST, Type.POST_UNCONDITIONAL]

@overload
def add_hook(
    func: str,
    type: _PreHookType,
    identifier: str,
    callback: _PreHookCallback,
) -> bool: ...
@overload
def add_hook(
    func: str,
    type: _PostHookType,
    identifier: str,
    callback: _PostHookCallback,
) -> bool: ...
@overload
def add_hook(
    func: str,
    type: Type,
    identifier: str,
    callback: _PreHookCallback | _PostHookCallback,
) -> bool:
    """
    Adds a hook which runs when an unreal function is called.

    Hook callbacks take four positional args. These are, in order:
        obj: The object the hooked function was called on.
        args: The arguments the hooked function was called with. Note that while
              this is mutable, modifying it will *not* modify the actual function
              arguments.
        ret: The return value of the unreal function, or the value of the previous
             return value override, if it has yet to run.
             Note that while there may be a `ReturnValue` property in the args
             struct, it is not necessarily correct, this always will be.
        func: The function which was called, bound to the same object. Can be used
              to re-call it.

    Pre-hooks can influence execution of the unreal function: they can block it from
    running, and/or overwrite it's return value.

    To block execution, return the sentinel `Block` type, (or an instance thereof),
    either by itself or as the first element of a tuple. Any other value will allow
    execution continue - suggest using Ellipsis when a value's required. If there
    are multiple hooks on the same function, execution is blocked if any hook
    requests it.

    To overwrite the return value, return it as the second element of a tuple. The
    the sentinel `Unset` type will prevent an override, while using Ellipsis will
    forward the previous value (rather than needing to copy it from `ret`). If there
    are multiple hooks on the same function, they will be run in an undefined order,
    where each hook is passed the previous override's value in `ret`, and the value
    returned by the final hook is what will be used.

    Post-hooks perform the same block/return override value processing, however as
    the function's already run, the effects are dropped. Overwriting the return
    value only serves to change what's passed in `ret` during any later hooks.

    Args:
        func: The name of the function to hook.
        type: Which type of hook to add.
        identifier: The hook identifier.
        callback: The callback to run when the hooked function is called.
    Returns:
        True if successfully added, false if an identical hook already existed.
    """

@overload
def add_bound_hook(
    func: BoundFunction,
    type: _PreHookType,
    identifier: str,
    callback: _PreHookCallback,
) -> bool: ...
@overload
def add_bound_hook(
    func: BoundFunction,
    type: _PostHookType,
    identifier: str,
    callback: _PostHookCallback,
) -> bool: ...
@overload
def add_bound_hook(
    func: BoundFunction,
    type: Type,
    identifier: str,
    callback: _PreHookCallback | _PostHookCallback,
) -> bool:
    """
    Adds a hook which is bound to a specific object.

    Acts as a wrapper around `add_hook`, inheriting all it's semantics. The given
    object is ignored when it comes to checking for identical hooks - hooking the
    same function on two different objects requires two different identifiers.

    Will not cause issues if the given object gets garbage collected, however
    calling code should remove the hook as soon as possible after this happens.

    Args:
        func: The bound function to hook.
        type: Which type of hook to add.
        identifier: The hook identifier.
        callback: The callback to run when the hooked function is called.
    Returns:
        True if successfully added, false if an identical hook (ignoring the object)
        already existed
    """

def has_hook(func: str, type: Type, identifier: str) -> bool:
    """
    Checks if a hook exists.

    Args:
        func: The name of the function to check.
        type: The type of hook to check.
        identifier: The hook identifier.
    Returns:
        True if a hook with the given details exists.
    """

def inject_next_call() -> None:
    """
    Makes the next unreal function call completely ignore hooks.

    Typically used to avoid recursion when re-calling the hooked function.
    """

def log_all_calls(should_log: bool) -> None:
    """
    Toggles logging all unreal function calls. Best used in short bursts for
    debugging.

    Args:
        should_log: True to turn on logging all calls, false to turn it off.
    """

def remove_hook(func: str, type: Type, identifier: str) -> bool:
    """
    Removes an existing hook.

    Args:
        func: The name of the function to remove hooks from.
        type: The type of hook to remove.
        identifier: The hook identifier.
    Returns:
        True if successfully removed, false if no hook with the given details exists.
    """
