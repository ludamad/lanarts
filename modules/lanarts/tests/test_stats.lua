require "utils"

local stats = require_relative "<stats"

local tests = {}

tests.test_stats_create = stats.stats_create

function main() 
    for k,v in pairs(tests) do
        print("Running test", k)
        v()
    end
end
