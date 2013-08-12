local stats = import "unstable.stats"

module( ..., lunit.testcase, package.seeall )

function test()
    lunit.assert(stats.stats_create())
end