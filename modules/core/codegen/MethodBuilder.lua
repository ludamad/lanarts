local MethodBuilder = newtype()

function MethodBuilder:init(typeinfo, ...)
	self.typeinfo = typeinfo
	self.signature = (", "):join{...}
	self.parts = {}
	self.indent = 1
end

function MethodBuilder:append(line)
    if type(line) == "table" then self:add(line)
    else local P = self.parts ; P[#P] = P[#P] .. line end
end
function MethodBuilder:add(line, ...)
    if type(line) == "table" then
        -- Actually a list of strings
        for real_line in values(line) do
            self:add(real_line, ...)
        end
    else
        if ... then line = line:format(...) end
        for i=1,self.indent do line = '    ' .. line end
        append(self.parts, line)
    end
end

function MethodBuilder:emit()
    return ("function(%s)\n%s\nend"):format(
        self.signature, ("\n"):join(self.parts)
    )
end

return MethodBuilder