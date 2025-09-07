# pyunrealsdk
[![Developer Discord](https://img.shields.io/static/v1?label=&message=Developer%20Discord&logo=discord&color=222)](https://discord.gg/VJXtHvh)

Python bindings for [unrealsdk](https://github.com/bl-sdk/unrealsdk), and embedded interpreter.

# Usage Overview
There are two ways of using the python sdk: via console command, or via the initialization script.

## Console Commands
The python sdk registers two custom console commands which you can use to execute python code.

`py` lets you run small snippets of python. By default, it executes one line at a time, stripping
any leading whitespace.

```py
py print(unrealsdk.find_all("PlayerController", exact=False))
```

You can also use heredoc-like syntax to execute multiline queries. This happens if the first two
non-whitespace characters are `<<` (which is invalid python syntax for a single line).
```py
py << EOF
obj = unrealsdk.find_object("WillowGameEngine", "Transient.WillowGameEngine_0")
if obj:
    print("Found engine:", obj)
else:
    print("Couldn't find engine!")
EOF
```

`pyexec` is useful for more complex scripts - it executes an entire file (relative to the game cwd).
Note that this is *not* running a python script in the traditional sense, it's instead more similar
to something like `eval(open(file).read())`. The interpreter is not restarted, and there's no way to
accept arguments into `sys.argv`.

## Initialization Script
If you want to make more permanent mods, you'll want to use the initialization script. By default
this is `__main__.py` in the same folder as the `pyunrealsdk.dll`. The initialization script is
automatically run after sdk initialization, so you can use it to import other files and generally
perform all your setup.

You can swap to a different initialization script by using setting `pyunrealsdk.init_script` in the
[unrealsdk configuration file](https://github.com/bl-sdk/unrealsdk/#configuration). If you do this
you may also want to set `pyunrealsdk.pyexec_root`, so that `pyexec` commands work from the same
folder. These are relative to the folder `pyunrealsdk.dll` is in (or you can supply absolute paths).

## Using SDK bindings
Once you've got code running, you probably want to setup some hooks - the sdk can run callbacks
whenever an unreal function is called, allowing you to interact with it's args, and mess with it's
execution.

```py
def on_main_menu(
    obj: unrealsdk.unreal.UObject,
    args: unrealsdk.unreal.WrappedStruct,
    func: unrealsdk.unreal.BoundFunction
) -> None:
    print("Reached main menu!")

unrealsdk.hooks.add_hook(
    "WillowGame.FrontendGFxMovie:Start",
    unrealsdk.hooks.Type.PRE,
    "main_menu_hook",
    on_main_menu
)
```

Alternatively, if you're simply using `pyexec` scripts, you might be able to find the objects you
want directly using `find_all` and/or `find_object`.

Once you have some unreal objects, you can access unreal properties through regular python attribute
access. This gets dynamically resolved to the relevant unreal property.

```py
paused = args.StartPaused

obj.MessagesOfTheDay[obj.MessageOfTheDayIdx].Body = "No MOTD today"

op_string = obj.BuildOverpowerPromptString(1, 10)
```

## Debugging
After you start writing some more complicated scripts, you'll probably want to get a debugger
working. To do this, the sdk has some integrations with [debugpy](https://github.com/microsoft/debugpy).

To use it:
1. Download and extract debugpy somewhere importable.

2. At the start of your initialization script, add the following:
   ```py
   import debugpy
   debugpy.listen(("localhost", 5678), in_process_debug_adapter=True)
   ```

3. Set `pyunrealsdk.debugpy = True` in the [unrealsdk configuration file](https://github.com/bl-sdk/unrealsdk/#configuration).

4. Attach using remote debugging with the debugger of your choice.

Note that the sdk disables the integration if it's unable to import debugpy on first use, meaning
you may not get away with manipulating `sys.path`. Instead, consider using [`._pth` files](https://docs.python.org/3/library/sys_path_init.html).

# Development
The sdk can be built with one of five different toolchains, each of which have a few different
configurations:

- MSVC
- Clang (Windows)
- Clang (Cross Compile) <sup>*</sup>
- MinGW <sup>*</sup>
- LLVM MinGW <sup>*</sup>

The toolchains with an asterix are all cross compiling toolchains. These all also have an associated
dev container, which is the recommended way of building them. The `clang-cross-*` presets in
particular hardcode a path assuming they're running in the container.

The sdk is always built as a Windows dll. To allow the cross compiling toolchains to link against a
Windows copy of Python, rather than using CMake's normal `FindPython` helpers, when configuring we
download a copy from Python's site. When integrating the sdk into your own projects, exactly which
version can be specified using the `EXPLICIT_PYTHON_ARCH` and `EXPLICIT_PYTHON_VERSION` variables.
The CMake presets already have these set if building locally.

To download the relevant python version, you need a copy of Python with requests on your path. When
cross compiling you also need `msiextract`, which is typically part of an `msitools` package. The
dev containers already have these set up.
```sh
pip install requests
python -c 'import requests'

# Linux only
apt install msitools # Or equivalent
msiextract --version 
```

Once you've got this all set up, to build the sdk:

1. Clone the repo (including submodules).
   ```sh
   git clone --recursive https://github.com/bl-sdk/pyunrealsdk.git
   ```

2. (OPTIONAL) Copy `postbuild.template`, and edit it to copy files to your game install directories.

3. Choose a preset, and run CMake. Most IDEs will be able to do this for you,
   ```
   cmake . --preset msvc-ue4-x64-debug
   cmake --build out/build/msvc-ue4-x64-debug
   ```

4. (OPTIONAL) Copy the python runtime files to the game's directory. At a minimum, you probably
   want these:
   ```
   python3.dll
   python3<version>.dll
   python3<version>.zip
   ```

   A CMake install will copy these files, as well as several other useful libraries, to the install
   dir for you.
   ```
   cmake --build out/build/msvc-ue4-x64-debug --target install
   ```

   As an alternative to this and step 2, you could point the CMake install dir directly at your
   game, so everything's automatically copied. This however will only work with one game at a time.

5. (OPTIONAL) If you're debugging a game on Steam, add a `steam_appid.txt` in the same folder as the
   executable, containing the game's Steam App Id.

   Normally, games compiled with Steamworks will call
   [`SteamAPI_RestartAppIfNecessary`](https://partner.steamgames.com/doc/sdk/api#SteamAPI_RestartAppIfNecessary),
   which will drop your debugger session when launching the exe directly - adding this file prevents
   that. Not only does this let you debug from entry, it also unlocks some really useful debugger
   features which you can't access from just an attach (i.e. Visual Studio's Edit and Continue).

# Python Extension Modules
The sdk provides some helpers to let you build extensions as python modules.

The sdk compiles to a shared module. As with unrealsdk itself, a few functions are exported to let
you interact with it's internal state from other modules (i.e. python extensions). This is done
transparently, you can just call the available C++ functions and they will automatically call into
the dll. Note that a lot of sdk code relates to hosting the interpreter, which is not exported.

The sdk also provides the `pyunrealsdk_add_module` CMake function, which creates a new target
linking against it, and building to a pyd. From this you also have full access to the libraries it
links against, so you can also call into any unrealsdk functions.
```cmake
cmake_minimum_required(VERSION 3.24)
project(my_project)

add_subdirectory(pyunrealsdk EXCLUDE_FROM_ALL)

pyunrealsdk_add_module(my_module my_module.cpp)
```
