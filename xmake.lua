add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

add_requires("levilamina 1.7.0", {configs = {target_type = "server"}})

add_requires("levibuildscript")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

target("PermCore") -- Change this to your mod name.
    add_rules("@levibuildscript/linkrule")
    add_cxflags( "/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
    add_defines("NOMINMAX", "UNICODE")
    add_packages("levilamina")
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("static")
    set_languages("c++20")
    set_symbols("debug")
    add_headerfiles("src/(perm_core/**.hpp)")
    add_files("src/**.cc")
    add_includedirs("src")
    add_defines("FMT_HEADER_ONLY=1") -- fix ODR errors

target("PermCoreTest")
    set_default(false)
    add_deps("PermCore")
    add_rules("@levibuildscript/modpacker")
    add_rules("@levibuildscript/linkrule")
    add_cxflags( "/EHa", "/utf-8", "/W4", "/w44265", "/w44289", "/w44296", "/w45263", "/w44738", "/w45204")
    add_defines("NOMINMAX", "UNICODE")
    add_packages("levilamina")
    set_exceptions("none") -- To avoid conflicts with /EHa.
    set_kind("shared")
    set_languages("c++20")
    set_symbols("debug")
    add_headerfiles("(test/**.hpp)")
    add_files("test/**.cc")
    add_includedirs("test", "src")
    add_defines("FMT_HEADER_ONLY=1") -- fix ODR errors