# loadx
loadx is a Lua module which provides a better load().

# Usage

This module provides the following functions:

### `loadx(chunk,  [, chunkname [, mode [, upvalues...]]])`

Loads a chunk.

This function is similar to Lua's native `load`, and has the following differences:

If the resulting function has upvalues, the upvalues are set to the values of `upvalues`,
if that parameter is given. The first upvalue, if not given, is set to the global environment.
(When you load a main chunk, the resulting function will always have exactly one upvalue,
the `_ENV` variable. However, when you load a binary chunk created from a function (e.g.
`string.dump`), the resulting function can have an arbitrary number of upvalues.)

See the native `load` for the definitions of `chunk`, `chunkname` and `mode`.

All caveats from the native `load` may apply.

See also:

- `load`:
[Lua 5.2](http://www.lua.org/manual/5.2/manual.html#pdf-load),
[Lua 5.3](http://www.lua.org/manual/5.3/manual.html#pdf-load).
- `string.dump`:
[Lua 5.2](http://www.lua.org/manual/5.2/manual.html#pdf-string.dump),
[Lua 5.3](http://www.lua.org/manual/5.3/manual.html#pdf-string.dump).

### `newupval()`

Creates an upvalue object.

Upvalue objects can be passed to loadx() in place of upvalue values. They allow you to share
upvalues between functions.

## Examples

### Shared upvalues

```lua
local loadx = require"loadx"

local up1 = loadx.newupval()
local up2 = loadx.newupval()
local env = loadx.newupval()

local UP1, UP2 -- dummies
local fc1 = string.dump(function(a, b, e) _ENV = e UP1 = a UP2 = b end) -- function code 1
local fc2 = string.dump(function() print(UP1[UP2]) end) -- function code 2

local set = loadx.loadx(fc1, nil, nil, env, up1, up2)
local prnt = loadx.loadx(fc2, nil, nil, env, up1, up2)

assert(not pcall(prnt)) -- should fail because we have no env
set(nil, nil, {print=print}) -- a, b, e, where e sets the _ENV
assert(not pcall(prnt)) -- should fail because we can't index nil
set({key="hi"}, "key", {print=print}) -- a, b, e. a[b] is "hi"
assert(pcall(prnt)) -- should print "hi"
```

## Compiling

To compile, on Linux, with gcc:

    gcc -fPIC loadx.c -shared -o loadx.so -I/path/to/lua/include/

This produces a `loadx.so` which you can then require().

Untested on Windows and OSX.

## Compatibility

loadx is fully compatible with Lua 5.2 and Lua 5.3. loadx is incompatible with Lua 5.1.
