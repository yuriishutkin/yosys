#!/usr/bin/env python

from pathlib import Path
import sys

assert sys.version_info.major >= 3, "only Python >= 3 is supported"

yosys_modules = [
    "backends/aiger",
    "backends/blif",
    "backends/btor",
    "backends/cxxrtl",
    "backends/edif",
    "backends/firrtl",
    "backends/intersynth",
    "backends/jny",
    "backends/json",
    "backends/rtlil",
    "backends/simplec",
    "backends/smt2",
    "backends/smv",
    "backends/spice",
    "backends/table",
    "backends/verilog",
    "frontends/aiger",
    "frontends/ast",
    "frontends/blif",
    "frontends/json",
    "frontends/liberty",
    "frontends/rpc",
    "frontends/rtlil",
    "frontends/verific",
    "frontends/verilog",
    "passes/cmds",
    "passes/equiv",
    "passes/fsm",
    "passes/hierarchy",
    "passes/memory",
    "passes/opt",
    "passes/pmgen",
    "passes/proc",
    "passes/sat",
    "passes/techmap",
    "passes/tests",
]


def read_file(filename, mode="r"):
    with open(filename, mode) as f:
        return f.read()


def extract_srcs(contents):
    s = contents.strip()
    obj_lines = [line for line in s.splitlines() if "OBJS" in line]
    srcs = list()
    for line in obj_lines:
        line = line.strip()
        if not line.startswith("OBJS"):
            continue
        line = line.removeprefix("OBJS").strip()
        if not line.startswith("+="):
            continue
        src = Path(line.removeprefix("+=").strip())
        if "pmgen" in str(src) or "verificsva" in src.name:
            continue
        src_cc = src.with_suffix(".cc")
        if src_cc.exists():
            srcs.append(str(src_cc))
        else:
            src_cpp = src.with_suffix(".cpp")
            if src_cpp.exists():
                srcs.append(str(src_cpp))
    return srcs


def write_module_cmakelists(dir, srcs):
    if len(srcs) == 0:
        return
    with open(f"{dir}/CMakeLists.txt", "w") as f:
        print("target_sources(yosys\n  PRIVATE", file=f)
        for src in srcs:
            src = src.split("/")[-1]
            print(f"  ${{CMAKE_CURRENT_SOURCE_DIR}}/{src}", file=f)
        print(")", file=f)


def write_dir_cmakelists(dir, mods):
    with open(f"{dir}/CMakeLists.txt", "w") as f:
        for mod in mods:
            print(f"add_subdirectory({mod})", file=f)


def main():
    yosys_srcs = [
        {"module": module, "srcs": extract_srcs(read_file(f"{module}/Makefile.inc"))}
        for module in yosys_modules
    ]
    base_dirs = dict()
    for mod in yosys_srcs:
        mod_name = mod["module"].split("/")[-1]
        print(f"found module {mod_name}")
        if len(mod["srcs"]) == 0:
            print(f"module {mod_name} has no sources")
            continue
        base_dir = "/".join(mod["module"].split("/")[:-1])
        if base_dirs.get(base_dir) is None:
            base_dirs[base_dir] = list()
        base_dirs[base_dir].append(mod_name)
        write_module_cmakelists(mod["module"], mod["srcs"])
    for dir, mods in base_dirs.items():
        write_dir_cmakelists(dir, mods)


if __name__ == "__main__":
    main()
