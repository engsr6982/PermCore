add_rules("mode.debug", "mode.release")

add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

add_requires("levilamina 1.7.0", {configs = {target_type = "server"}})

add_requires("levibuildscript")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

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
    add_headerfiles("./(test/**.hpp)")
    add_files("./test/**.cc")
    add_includedirs("./test", "./src")
    add_defines("FMT_HEADER_ONLY=1") -- fix ODR errors
        after_build(function(target)
        local project_dir =  os.projectdir()
        local assets_dir = path.join(project_dir, "assets")
        local lang_assets_dir = path.join(assets_dir, "lang")

        local bin_dir = path.join(project_dir, "bin")
        local lang_bin_dir = path.join(bin_dir, target:name(), "lang")

        os.mkdir(lang_bin_dir)
        os.cp(path.join(lang_assets_dir, "**"), lang_bin_dir)
    end)