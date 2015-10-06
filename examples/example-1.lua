-- Shared Upvalues example on the README

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
