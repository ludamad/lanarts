local stats = import "unstable.Stats"

function TestCases.stats_create()
    lunit.assert(stats.stats_create())
end