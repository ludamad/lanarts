local yaml = import "core.yaml"

--local tags = {
--    Aptitude
--}
--
print "Hello"
local y = yaml.load(file_as_string(path_resolve "aptitudes.yaml"))
pretty_print(y)