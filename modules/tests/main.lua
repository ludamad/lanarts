import 'core.utils'
import '@lunit'

local tests = {
    "tests.lanarts.test_stats",
    "tests.lanarts.test_object_relations",
    "tests.core.test_SerializeBuffer"
}

lunit.main{'--', unpack(tests)}