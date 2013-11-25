function TestCases.test()
    local Stats = import "@Stats"
    local vec = Stats.AptVec.create(1,2,3,4)
    local apts = Stats.Aptitudes.create(vec)
    local stats1 = Stats.Stats.create(apts)
    local stats2 = Stats.Stats.create(apts)
    local context = Stats.StatContext.create(stats1, stats2)
    print(context)
end