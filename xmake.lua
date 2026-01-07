
if not has_config("vs_runtime") then
    set_runtimes("MD")
end

includes("./static_lib.lua")
includes("./test_lib.lua")

-- add_defines("LL_I18N_COLLECT_STRINGS")