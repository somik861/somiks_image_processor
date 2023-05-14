# What and why
configs are stored as an *.json file and contains information about options that can be passed to a specific codec.

More specifically, config root structure is dictionary and contains keys:
* options
    * list of options that can be specified
    * option:
        * description: "Option description"
        * type: "int" | "float" | "text" | "choice" | "checkbox"
        * values: ["a", "b", "c"] (only if type is "choice")
        * range: [1, 100] (numeric range if type is "float" or "int", size range if type is "text")
        * default: 0
        * options:
            * list of options; only if type is "checkbox"

* extensions
    * list of extension to match
    * extension:
        * suffix: ".tiff"
        * type: "plain" | "regex" (defaults to "plain" if ommited)
