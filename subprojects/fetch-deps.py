#!/usr/bin/env python3
import os, subprocess
# https://stackoverflow.com/a/1432949
os.chdir(os.path.dirname(os.path.abspath(__file__)))

RESET="\033[0m"; RED="\033[31m"; BLU="\033[34m"; BOLD="\033[1m"

def run_cmd(*args, **kwargs):
    print("cmd: " + " ".join(args[0]))
    result = subprocess.run(*args, **kwargs)
    return result

def fetch_dependency(directory: str, giturl: str, revision: str):
    print(f"\n{BOLD}Fetching {giturl} at {directory}.{RESET}")
    if not os.path.isdir(os.path.join(directory, ".git")):
        cmd = ["git", "clone", "--filter=blob:none", "--no-checkout", giturl, directory]
        result = run_cmd(cmd)
        if result.returncode != 0:
            print(f"{RED}Couldn't fetch {giturl}{RESET}")
            return
    run_cmd(["git", "-C", directory, "fetch"])
    run_cmd(["git", "-C", directory, "checkout", revision])

def apply_patch(directory: str, patch: str):
    patch = os.path.relpath(patch, directory)
    result = run_cmd(["git", "-C", directory, "apply", "--reverse", "--check", patch])
    if result.returncode == 0:
        print(f"{BLU}Patch applied {patch}{RESET}")
        return
    result = run_cmd(["git", "-C", directory, "apply", patch])
    if result.returncode == 0:
        print(f"{BLU}Patch applied {patch}{RESET}")
    else:
        print(f"{RED}Couldn't apply patch {patch}{RESET}")

# Dependencies:

fetch_dependency(
"woycontainer",
"https://github.com/woynert/woycontainer",
"4b3e0bf61cd4529429aa1f5cf4b84643038dcc2c")

fetch_dependency(
"cwalk",
"https://github.com/likle/cwalk",
"f45a23a13abf39d94b347d7c83810eca26a5a8d0")

fetch_dependency(
"mickstr/mickstr",
"https://github.com/Woynert/mickjc750-str",
"e1646f2bd43c5f47500d74695d7643b3b346b482")

fetch_dependency(
"glfw",
"https://github.com/glfw/glfw",
"d9d6f0f1f967807ffade6598ea9a631ebaf37a56") # Tag 3.5.1

fetch_dependency(
"silk/silk",
"https://github.com/joleksia/Silk",
"c668fc73e3ab1f726de870fd2a6783788d232929")
