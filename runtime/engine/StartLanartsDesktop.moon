Engine_internal = require "core.Engine"
argparse = require "argparse"
Display = require "core.Display"
yaml = require "yaml"

settings = () ->


main = (raw_args) ->
    parser = argparse("lanarts", "Run lanarts.")
    parser\option "--debug", "Attach debugger."
    parser\option "--nofilter", "Do not filter Lua error reporting of noise."
    args = parser\parser(raw_args)

    -- (1) Handle debug options
    if args.debug
        debug = debug.attach_debugger()

    -- (2) Handle error reporting options
    ErrorReporting = require "ErrorReporting"
    if args.nofilter


    // Width 0 resolves to monitor width:
    Size screen_size = ldraw::screen_size();
    if (settings.view_width == 0) {
        settings.view_width = screen_size.w;
    }
    // Height 0 resolves to monitor height:
    if (settings.view_height == 0) {
        settings.view_height = screen_size.h;
    }

return main
