local yaml = import "core.yaml"

print "Hello"
local y = yaml.load(file_as_string(path_resolve "earlygame.yaml"))
pretty(y)