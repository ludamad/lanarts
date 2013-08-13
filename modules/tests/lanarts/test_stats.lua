local stats = import "unstable.stats"

function TestCases.stats_create()
    lunit.assert(stats.stats_create())
end