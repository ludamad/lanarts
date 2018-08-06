yaml = require "yaml"

settings_save = (settings) ->
    -- Injected into settings in further engine runs:
    {:time_per_step, :username, :class_type, :frame_action_repeat, :regen_on_death, :volume} = settings
    yaml_str = yaml.dump({:time_per_step, :username, :class_type, :frame_action_repeat, :regen_on_death})
    -- ensure_directory("saves")
    file_dump_string("saves/saved_settings.yaml", yaml_str)

-- Parse settings object from settings yaml file
parse_settings = (settings_file) ->
    log_info "Parsing #{settings_file}"
    yaml_text = file_as_string(settings_file)
    if not yaml_text
        error("Fatal error: #{settings_file} not found, the game is probably being loaded from the wrong place.")
    raw_settings = try {
        do: () -> yaml.load(yaml_text)
        catch: (err) -> error("Invalid YAML syntax:\n#{err}")
    }
    settings = {key, value for {key, value} in *raw_settings}
    return settings

settings_load = (settings_file) ->
    settings = parse_settings(settings_file)
    if file_exists("saves/saved_settings.yaml")
        saved_settings = parse_settings("saves/saved_settings.yaml")
        for k, v in pairs(saved_settings)
            settings[k] = v
    return settings

return {:settings_save, :settings_load}
