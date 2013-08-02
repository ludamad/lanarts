function speed_description(time_per_step)
    local text = "Slow"

    if time_per_step <= 10 then
        text = "Very Fast"
    elseif time_per_step <= 12 then
        text = "Fast"
    elseif time_per_step <= 14 then
        text = "Normal"
    end
    return text
end
