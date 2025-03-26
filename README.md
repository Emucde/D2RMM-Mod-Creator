# D2RMM-Mod-Creator

This tool creates a D2RMM-compatible mod from multiple ".txt" files. Here's how it works:

1. Place your source files (e.g., "cubemain.txt", "itemstatcost.txt", "properties.txt") in the same folder as "convert_modfiles.exe".
2. Run "convert_modfiles.exe".
3. The tool generates two files: "mod.js" and "mod.json".
4. Create a new folder in the D2RMM mod folder (e.g., "Mod Merge") and place the generated "mod.js" and "mod.json" files inside it.
5. Open D2RMM, go to the "Mods" tab, and ensure the newly created "Mod Merge" folder is at the top of the mod list. This ensures your merged mod takes priority over other mods.
6. Click "Install Mods" in D2RMM to apply your changes.

**Key Benefits:**
- Ideal for mod developers who create hardcoded mods using ".txt" files and want to convert them into D2RMM-compatible files.
- Allows integration with other D2RMM mods.
- Works with open-source mods only.
- Supports ".txt" files, not binary (".bin") files.

**Important Considerations:**
- The tool cannot handle strings or UI elements—only ".txt" files are supported.
- It has not been extensively tested, so there may be edge cases where it doesn't work as expected.
- The tool may not work correctly if your source files are corrupted or contain unusual characters.
- For added peace of mind, you may want to scan "convert_modfiles.exe" with a virus scanner before use.
- The source code for this tool is also attached for transparency and review.

**Note:** This tool is designed for use with D2RMM (Diablo 2 Resurrected Mod Manager).