local StatusType = import "@StatusType"
local ContentUtils = import "@stats.ContentUtils"

local M = nilprotect {} -- Submodule

function M.status_type_define(args)
    if args.on_draw then
        args.on_draw = ContentUtils.derive_on_draw(args)
    end
    return StatusType.define(args)
end

return M