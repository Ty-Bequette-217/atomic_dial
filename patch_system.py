import json
import os
import glob
packages_dir = r"C:\Users\cocou\.platformio\packages"
for p in glob.glob(os.path.join(packages_dir, "*-earlephilhower", "package.json")):
    with open(p, "r") as f:
        data = json.load(f)
    systems = data.get("system", [])
    if isinstance(systems, str):
        systems = [systems]
    if "windows_amd64" in systems and "windows_arm64" not in systems:
        systems.append("windows_arm64")
        data["system"] = systems
        with open(p, "w") as f:
            json.dump(data, f, indent=2)
        print("Updated", p)
