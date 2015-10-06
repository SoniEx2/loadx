# loadx
loadx is a Lua module which provides a better load().

# Usage

This module provides the following functions:

### `loadx(chunk,  [, chunkname [, mode [, upvalues...]]])`

Loads a chunk.

This function is similar to Lua's native `load`, and has the following differences:

If the resulting function has upvalues, the upvalues are set to the values of `upvalues`, if that parameter is given. For Lua 5.2+, the first upvalue, if not given, is set to the global environment. (For Lua 5.2+, when you load a main chunk, the resulting function will always have exactly one upvalue, the `_ENV` variable. However, when you load a binary chunk created from a function (e.g. `string.dump`), the resulting function can have an arbitrary number of upvalues.)

The string `mode` controls whether the chunk can be text or binary (that is, a precompiled chunk). It may be the string `"b"` (only binary chunks), `"t"` (only text chunks), or `"bt"` (both binary and text). The default is `"bt"`. **This argument has no effect when running under Lua 5.1. For Lua 5.1, it is only kept to provide a uniform API across Lua versions.**

See the native `load` for the definitions of `chunk` and `chunkname`.

All caveats from the native `load` may apply.

See also:

- `load`:
[Lua 5.1](http://www.lua.org/manual/5.1/manual.html#pdf-load) ([`loadstring`](http://www.lua.org/manual/5.1/manual.html#pdf-loadstring)),
[Lua 5.2](http://www.lua.org/manual/5.2/manual.html#pdf-load),
[Lua 5.3](http://www.lua.org/manual/5.3/manual.html#pdf-load).
- `string.dump`:
[Lua 5.1](http://www.lua.org/manual/5.1/manual.html#pdf-string.dump),
[Lua 5.2](http://www.lua.org/manual/5.2/manual.html#pdf-string.dump),
[Lua 5.3](http://www.lua.org/manual/5.3/manual.html#pdf-string.dump).

## Compiling

To compile, on Linux, with gcc:

    gcc -fPIC loadx.c -shared -o loadx.so -I/path/to/lua/include/

This produces a `loadx.so` which you can then require().

Untested on Windows and OSX.

## Compatibility

loadx is fully compatible with Lua 5.2 and Lua 5.3. loadx is partially compatible with Lua 5.1, lacking support for the `mode` argument.
