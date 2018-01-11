define(lac_config_path, configuration)dnl
define(lac_include_macros,[builtin(include,lac_config_path/[$1])])dnl
lac_include_macros(lac.m4)

