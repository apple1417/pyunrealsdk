# Changelog

## Upcoming
- The `pyunrealsdk.init_script` and `pyunrealsdk.pyexec_root` config options are now relative to the
  folder containing the `pyunrealsdk.dll`. Previously, they were relative to the cwd, which could
  cause issues if it changed.

## v1.8.0
- Added `WeakPointer.replace`, to modify a pointer in-place.

- Trying to overwrite the return value of a void function will now return a more appropriate error.

- The type hinting of `WrappedArray`s now default to `WrappedArray[Any]`, no generic required.

- Upgraded to support unrealsdk v2 - native modules can expect some breakage. The most notable
  effect this has on Python code is a number of formerly read-only fields on core unreal types have
  become read-write.

### unrealsdk v2.0.0
For reference, the unrealsdk v2.0.0 changes this includes are:

> - Now supports Borderlands 1. Big thanks to Ry for doing basically all the reverse engineering.
> 
> - Major refactor of the core unreal types, to cleanly allow them to change layouts at runtime. All
>   core fields have changed from members to zero-arg methods, which return a reference to the
>   member. A few classes (e.g. `UProperty` subclasses) previous had existing methods to deal with
>   the same problem, these have all been moved to the new system.
>   
>   Clang is able to detect this change, and gives a nice error recommending inserting brackets at
>   the right spot.
>   
> - Removed the `UNREALSDK_UE_VERSION` and `UNREALSDK_ARCH` CMake variables, in favour a new merged
>   `UNREALSDK_FLAVOUR` variable.
> 
> - Removed the (optional) dependency on libfmt, `std::format` support is now required.
> 
> - Console commands registered using `unrealsdk::commands::NEXT_LINE` now (try to) only fire on
>   direct user input, and ignore commands send via automated means.
> 
> - Fixed that assigning an entire array, rather than getting the array and setting it's elements,
>   would likely cause memory corruption. This was most common when using an array of large structs,
>   and when assigning to one which was previously empty.
> 
> - Made `unrealsdk::memory::get_exe_range` public.

## v1.7.0

- Added `WrappedArray.emplace_struct`, to construct structs in place. This is more efficient than
  calling `arr.insert(pos, unrealsdk.make_struct(...))`.

  [dc515cdc](https://github.com/bl-sdk/unrealsdk/commit/dc515cdc)

- Added `unrealsdk.unreal.IGNORE_STRUCT`, a sentinel value which can be assigned to any struct, but
  which does nothing. This is most useful when a function has a required struct arg.

  [6c0b58ee](https://github.com/bl-sdk/unrealsdk/commit/6c0b58ee)
  
- Added support for sending property changed events. This is typically best done via the
  `unrealsdk.unreal.notify_changes` context manager.

  [97e3e0c2](https://github.com/bl-sdk/unrealsdk/commit/97e3e0c2)

- Fixed that it was possible for the `unrealsdk` module in the global namespace to get replaced, if
  something during the init script messed with `sys.modules`. It is now imported during
  initialization.

  [91cfee4b](https://github.com/bl-sdk/unrealsdk/commit/91cfee4b)

### unrealsdk v1.8.0
For reference, the unrealsdk v1.8.0 changes this includes are:

> - Added support for sending property changed events, via `UObject::post_edit_change_property` and
>   `UObject::post_edit_change_chain_property`.
>   
>   [a6040da4](https://github.com/bl-sdk/unrealsdk/commit/a6040da4)
> 
> - Made the error message when assigning incompatible array types more clear.
> 
>   See also https://github.com/bl-sdk/unrealsdk/issues/60 .
> 
>   [6222756c](https://github.com/bl-sdk/unrealsdk/commit/6222756c)
>
> - Fixed checking the setting `exe_override` rather than the full `unrealsdk.exe_override`, like
>   how it was documented / originally intended.
>
>   [3010f486](https://github.com/bl-sdk/unrealsdk/commit/3010f486)

## v1.6.0

- `WrappedStruct` now supports being copied via the `copy` module.

  [2ce96e52](https://github.com/bl-sdk/pyunrealsdk/commit/2ce96e52)

- Fixed that `WrappedArray.index` would not check the last item in the array. It also now accepts
  start/stop indexes beyond the array bounds, like `list.index` does.

  [10bdc130](https://github.com/bl-sdk/pyunrealsdk/commit/10bdc130)

- Hook return values and array access now have the same semantics as normal property accesses. In
  practice this means:

  - Getting an enum property will convert it to a python `IntFlag` enum (rather than an int).
  - Setting an array property will accept any sequence (rather than just wrapped arrays).
  
  All other property types had the same semantics already, so this is backwards compatible.

  [c52a807e](https://github.com/bl-sdk/pyunrealsdk/commit/c52a807e),
  [16ac1711](https://github.com/bl-sdk/pyunrealsdk/commit/16ac1711)

- Added a `_get_address` method to `WrappedArray`, `WrappedMulticastDelegate`, and `WrappedStruct`.

  [1b3e9686](https://github.com/bl-sdk/pyunrealsdk/commit/1b3e9686)


### unrealsdk v1.7.0
For reference, the unrealsdk v1.7.0 changes this includes are:

> - `unrealsdk::unreal::cast` now copies the const-ness of its input object to its callbacks.
> 
>   [779d75ea](https://github.com/bl-sdk/unrealsdk/commit/779d75ea)
> 
> - Reworked `PropertyProxy` to be based on `UnrealPointer` (and reworked it too). This fixes some
>   issues with ownership and possible use after frees.
>   
>   *This breaks binary compatibility*, though existing code should work pretty much as is after a
>   recompile.
>   
>   [49bff4a4](https://github.com/bl-sdk/unrealsdk/commit/49bff4a4)

## v1.5.2

### unrealsdk v1.6.1
For reference, the unrealsdk v1.6.1 changes this includes are:

> - Handled `UClass::Interfaces` also having a different offset between BL2 and TPS.

## v1.5.1
- Changed type hinting of `unrealsdk.find_all` to return an `Iterable[UObject]`, instead of
  `Iterator[UObject]`. This mirrors what was actually happening at runtime.

  [fbe877ef](https://github.com/bl-sdk/pyunrealsdk/commit/fbe877ef)

### unrealsdk v1.6.0
For reference, the unrealsdk v1.6.0 changes this includes are:

> - Handled `UStruct` differing in size between BL2 and TPS.
> 
>   This affects all members on it's subclasses - `UClass::ClassDefaultObject`, `UClass::Interfaces`,
>   `UFunction::FunctionFlags`, `UFunction::NumParams`, `UFunction::ParamsSize`,
>   `UFunction::ReturnValueOffset`, and `UScriptStruct::StructFlags` have all now changed to methods
>   which return a reference.
> 
>   [72579c18](https://github.com/bl-sdk/unrealsdk/commit/72579c18)
> 
> - Fixed all BL3 console output being treated as console commands instead.
> 
>   [1432408f](https://github.com/bl-sdk/unrealsdk/commit/1432408f)

## v1.5.0

- Deprecated `unrealsdk.hooks.inject_next_call` in favour of a new
  `unrealsdk.hooks.prevent_hooking_direct_calls` context manager.

  [59f474fe](https://github.com/bl-sdk/pyunrealsdk/commit/59f474fe)

- The `WeakPointer` constructor can now be called with no args, to create a null pointer.

  [59f474fe](https://github.com/bl-sdk/pyunrealsdk/commit/59f474fe)

- As with unrealsdk, reworked the configuration system to no longer rely on environment variables.
  All sdk configuration is now also done through the `unrealsdk.toml`.

  The full contents of the unrealsdk config are parsed and exposed to Python in `unrealsdk.config`.

  [ecde0a83](https://github.com/bl-sdk/pyunrealsdk/commit/ecde0a83)

- `unrealsdk.construct_object` now throws a `RuntimeError` instead of silently returning `None` when
  constructing the object fails. This is how the type hints already assumed it worked.

  [264634e2](https://github.com/bl-sdk/pyunrealsdk/commit/264634e2)

### unrealsdk v1.5.0
For reference, the unrealsdk v1.5.0 changes this includes are:

> - Completely reworked the configuration system.
> 
>   Environment variables and the `unrealsdk.env` are no longer used, due to issues with them not fully
>   propagating within the same process. The new configuration now uses an `unrealsdk.toml` instead.
>   
>   Also added support for a user specific override file - `unrealsdk.user.toml`. This allows projects
>   to ship their own `unrealsdk.toml`, without overwriting user's settings on update.
> 
>   [4daecbde](https://github.com/bl-sdk/unrealsdk/commit/4daecbde)
> 
> - `unrealsdk::hook_manager::inject_next_call` is now thread local.
> 
>   [427c8734](https://github.com/bl-sdk/unrealsdk/commit/427c8734)
> 
> - Fixed that `unrealsdk::commands::has_command` and `unrealsdk::commands::remove_command` were case
>   sensitive, while `unrealsdk::commands::add_command` and the callbacks were not. Commands should be
>   now be case insensitive everywhere.
> 
>   [b641706d](https://github.com/bl-sdk/unrealsdk/commit/b641706d)
> 
> - Fixed that the executed command message of custom sdk commands would not appear in console if you
>   increased the minimum log level, and that they may have appeared out of order with respects to
>   native engine messages.
> 
>   [b652da13](https://github.com/bl-sdk/unrealsdk/commit/b652da13)
> 
> - Added an additional console command hook in BL2, to cover commands not run directly via console.
> 
>   [1200fca4](https://github.com/bl-sdk/unrealsdk/commit/1200fca4)
> 
> - Renamed the `unrealsdk.locking_process_event` (previously `UNREALSDK_LOCKING_PROCESS_EVENT`)
>   setting to `unrealsdk.locking_function_calls`, and expanded it's scope to cover all function
>   calls. This fixes a few more possibilities for lockups.
> 
>   [bebaeab4](https://github.com/bl-sdk/unrealsdk/commit/bebaeab4)
> 
> - Trying to set a struct, array, or multicast delegate to itself is now a no-op, and prints a
>   warning.
> 
>   [8a98db1f](https://github.com/bl-sdk/unrealsdk/commit/8a98db1f)
> 
> - The console key will now also be overwritten if it was previously set to `Undefine`.
> 
>   [631fa41e](https://github.com/bl-sdk/unrealsdk/commit/631fa41e)

## v1.4.0

- Fixed weak pointer type hinting to allow for null pointers. This always worked at runtime.

  [1cbded47](https://github.com/bl-sdk/pyunrealsdk/commit/1cbded47)
  
- Added support for Delegate and Multicast Delegate properties.

  [04d47f92](https://github.com/bl-sdk/pyunrealsdk/commit/04d47f92),
  [2876f098](https://github.com/bl-sdk/pyunrealsdk/commit/2876f098)

- Added a repr to `BoundFunction`, as these are now returned by delegates.

  [22082579](https://github.com/bl-sdk/pyunrealsdk/commit/22082579)

- The `WrappedStruct` constructor can now be used to construct function arg structs, like what's
  passed to a hook. This *does not* also apply to `unrealsdk.make_struct`, since function names
  conflict far more often.
  
  [5e2fd0df](https://github.com/bl-sdk/pyunrealsdk/commit/5e2fd0df)

- Removed the suggestion to return `Ellipsis` in hooks when overwriting a return value but not
  blocking execution. This still works at runtime, but is no longer present in the type hinting,
  since it made `Ellipsis` be typed as a valid return in several places it shouldn't have been, and
  it's an obscure use case to begin with.

  [1711c1a8](https://github.com/bl-sdk/pyunrealsdk/commit/1711c1a8)

### unrealsdk v1.4.0
For reference, the unrealsdk v1.4.0 changes this includes are:

> - Fixed that UE3 `WeakPointer`s would always return null, due to an incorrect offset in the
>   `UObject` header layout.
>
>   [aca70889](https://github.com/bl-sdk/unrealsdk/commit/aca70889)
>
> - Added support for Delegate and Multicast Delegate properties.
>
>   [4e17d06d](https://github.com/bl-sdk/unrealsdk/commit/4e17d06d),
>   [270ef4bf](https://github.com/bl-sdk/unrealsdk/commit/270ef4bf)
>
> - Changed `unrealsdk::hook_manager::log_all_calls` to write to a dedicated file.
>
>   [270ef4bf](https://github.com/bl-sdk/unrealsdk/commit/270ef4bf)
> 
> - Fixed missing all `CallFunction` based hooks in TPS - notably including the say bypass.
>
>   [011fd8a2](https://github.com/bl-sdk/unrealsdk/commit/270ef4bf)
>
> - Added the offline mode say crash fix for BL2+TPS as a base sdk hook.
>
>   [2d9a36c7](https://github.com/bl-sdk/unrealsdk/commit/270ef4bf)

## v1.3.0

Also see the unrealsdk v1.3.0 changelog [here](https://github.com/bl-sdk/unrealsdk/blob/master/changelog.md#v130).

- Added bindings for the new classes introduced in unrealsdk v1.3.0 - `WeakPointer`,
  `ULazyObjectProperty`, `USoftObjectProperty`, and `USoftClassProperty`

  [18294df4](https://github.com/bl-sdk/pyunrealsdk/commit/18294df4),
  [7e724f1e](https://github.com/bl-sdk/pyunrealsdk/commit/7e724f1e),
  [6558e1cc](https://github.com/bl-sdk/pyunrealsdk/commit/6558e1cc)

- Fixed that hooks could not always be removed after adding, or that they might not always fire.

  [unrealsdk@227a93d2](https://github.com/bl-sdk/unrealsdk/commit/227a93d2)

## v1.2.0

Also see the unrealsdk v1.2.0 changelog [here](https://github.com/bl-sdk/unrealsdk/blob/master/changelog.md#v120).

- Added bindings for the new classes introduced in unrealsdk v1.2.0 - `UByteAttributeProperty`,
  `UComponentProperty`, `UFloatAttributeProperty`, `UIntAttributeProperty`, and
  `UByteProperty::Enum`.

  [ab211486](https://github.com/bl-sdk/pyunrealsdk/commit/ab211486)

- Getting a byte property which has an associated enum will now return the appropriate Python enum,
  in the same way as an enum property does. Byte properties without an enum still return an int.

  [ab211486](https://github.com/bl-sdk/pyunrealsdk/commit/ab211486)

- Fixed that it was impossible to set Python properties on unreal objects.

  [8b75fbbf](https://github.com/bl-sdk/pyunrealsdk/commit/8b75fbbf),
  [730a813f](https://github.com/bl-sdk/pyunrealsdk/commit/730a813f)

- Changed the log level specific printers, `unrealsdk.logging.error` et al., to each use their own,
  logger objects rather than modifying `sys.stdout` in place.

  [285e276a](https://github.com/bl-sdk/pyunrealsdk/commit/285e276a),
  [34e190b6](https://github.com/bl-sdk/pyunrealsdk/commit/34e190b6)

- Added the `PYUNREALSDK_PYEXEC_ROOT` env var to let you customize where `pyexec` commands are run
  relative to.

  [fdbda407](https://github.com/bl-sdk/pyunrealsdk/commit/fdbda407)

- Fixed that a fully qualified `unrealsdk.find_class` would not allow subclasses. This was most
  notable with blueprint generated classes.

  [unrealsdk@643fb46e](https://github.com/bl-sdk/unrealsdk/commit/643fb46e)

- Updated various docstrings and type stubs to be more accurately.

  [d66295ef](https://github.com/bl-sdk/pyunrealsdk/commit/d66295ef),
  [0df05cea](https://github.com/bl-sdk/pyunrealsdk/commit/0df05cea),
  [285e276a](https://github.com/bl-sdk/pyunrealsdk/commit/285e276a)

- Restructured CMake to allow you to define the Python version to link against directly within it,
  similarly to unrealsdk.

  ```cmake
  set(UNREALSDK_ARCH x64)
  set(UNREALSDK_UE_VERSION UE4)
  set(EXPLICIT_PYTHON_ARCH win64)
  set(EXPLICIT_PYTHON_VERSION 3.12.3)

  add_subdirectory(libs/pyunrealsdk)
  ```
  [abca72b3](https://github.com/bl-sdk/pyunrealsdk/commit/abca72b3)

- Release the GIL during unreal function calls, to try avoid a deadlock when running with
  `UNREALSDK_LOCKING_PROCESS_EVENT`.

  [31fdb4ee](https://github.com/bl-sdk/pyunrealsdk/commit/31fdb4ee)

- Upgraded pybind.

  [b1335304](https://github.com/bl-sdk/pyunrealsdk/commit/b1335304)

## v1.1.1
- Updated CI and stubs to Python 3.12

  [0d0cbce9](https://github.com/bl-sdk/pyunrealsdk/commit/0d0cbce9)

## v1.1.0

Also see the unrealsdk v1.1.0 changelog [here](https://github.com/bl-sdk/unrealsdk/blob/master/changelog.md#v110).

- Add support for [debugpy](https://github.com/microsoft/debugpy)., to let it trigger python
  breakpoints on other threads. See the new [Debugging](Readme.md#Debugging) section in the readme
  for more.

  [082f1252](https://github.com/bl-sdk/pyunrealsdk/commit/082f1252)

- Add bindings for:
  - `unrealsdk.load_package`

    [82e56fe4](https://github.com/bl-sdk/pyunrealsdk/commit/82e56fe4)

  - `UObject._path_name` - which does not include the class, in contrast to `__repr__`.

    [363fbe48](https://github.com/bl-sdk/pyunrealsdk/commit/363fbe48)

  - `UStruct._superfields`

    [2f5dca2d](https://github.com/bl-sdk/pyunrealsdk/commit/2f5dca2d)


- Add const getters to `StaticPyObject`.

  [8edc4580](https://github.com/bl-sdk/pyunrealsdk/commit/8edc4580)

- Add support for building using standard GCC-based MinGW. This is not tested in CI however, as it
  requires a newer version than that available in Github Actions.

  [d692f0e9](https://github.com/bl-sdk/pyunrealsdk/commit/d692f0e9),
  [2727b17c](https://github.com/bl-sdk/pyunrealsdk/commit/2727b17c)

- Update stub linting, and do a few minor fixups.

  [84aedc62](https://github.com/bl-sdk/pyunrealsdk/commit/84aedc62),
  [a228b56e](https://github.com/bl-sdk/pyunrealsdk/commit/a228b56e)


## v1.0.0
- Initial Release
