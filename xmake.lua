add_rules("mode.debug", "mode.release")

-- Packages
add_requires("unordered_dense")
add_requires("fmt", { system = true })

target("solver.elf")
    set_rundir("$(projectdir)")
    set_languages("cxx20")
    set_kind("binary")
    -- CXX Flags
    set_warnings("all", "error")
    if is_mode("debug") then 
        add_cxxflags("-fstandalone-debug")
        add_cxflags("-fsanitize=address")
        add_ldflags("-fsanitize=address")
    end 
    
    if is_mode("release") then
        set_optimize("fastest") 
        add_cxxflags("-flto")
    end
    add_includedirs("include")
    add_files("src/*.cpp|checker.cpp|test.cpp")
    add_packages("unordered_dense")
    add_packages("fmt")

target("check.elf")
    set_rundir("$(projectdir)")
    set_languages("cxx20")
    set_kind("binary")
    -- CXX Flags
    set_warnings("all", "error")
    if is_mode("debug") then 
        add_cxxflags("-fstandalone-debug")
        add_cxflags("-fsanitize=address")
        add_ldflags("-fsanitize=address")
    end 
    
    if is_mode("release") then
        set_optimize("fastest") 
        add_cxxflags("-flto")
    end
    add_includedirs("include")
    add_files("src/*.cpp|main.cpp|test.cpp")
    add_packages("unordered_dense")
    add_packages("fmt")