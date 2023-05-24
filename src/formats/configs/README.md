# What and why
configs are stored as an *.json file and contains information about options that can be passed to a specific codec.

More specifically, config root structure is dictionary and contains keys:
* options
    * list of options that can be specified
    * option:
        * type: "header" | "int" | "float" | "text" | "choice" | "checkbox" | "subsection"
        * text: "Option description or header text"
        * values: ["a", "b", "c"] (only if type is "choice")
        * range: [1, 100] (numeric range if type is "float" or "int", size range if type is "text")
        * default: ? value of given valid entry ?
        * options:
            * list of options; only if type is "subsection"
        * var_name: unique identifier, that would be used in C++ code to identify the option (not required for "header")

* extensions
    * list of extension to match
    * extension:
        * suffix: ".tiff"
        * type: "plain" | "regex" (defaults to "plain" if ommited)

* ouptut_extension: string (without '.')
