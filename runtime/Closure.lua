local Closure = newtype {}

function Closure:init(args, func)
    self.args = args
    self.func = func
end

function Closure:__call(...)
    self.func(self.args, ...)
end

return Closure
