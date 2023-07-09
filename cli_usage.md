# CLI usage documentation

## Few introductory words
The CLI is still in development and it might be changed in the future. To allow for maximum power, the cli options can
be provided and combined in many ways. Altough there will be human-friendly error messages for some invalid inputs, most of the time the
program will just end with an exception if invalid input is provided.
Therefore, I strongly recommend reading this manual before usage and to strictly obey its rules.

## Example help output
``` pwsh
PS> ssimp --help
Usage: ./ssimp input_path [output_path] [--help] [--recurse]
        [--preset <preset.json>]
        [--loading_options <lopt.json>] [--loading_opt_string <string>]
        [--format <string> [--saving_options <sopt.json>] [--saving_opt_string <string>]]
        [{--algorithm <string>}... [--algo_options <algo_options.json>] [--algo_opt_string <string>]]

Generic options:
  -h [ --help ]            produce help message
  --debug                  produce debug messages
  --print_info             only print information about image
  --loading_options arg    path to json file containing options for loading
                           files
  --loading_opt_string arg json string (can be used instead of loading_options)
  -f [ --format ] arg      output format
  -r [ --recurse ]         Recurse into subdirectories
  --saving_options arg     path to json file containing saving options for
                           format
  --saving_opt_string arg  json string (can be used instead of saving_options)
  -a [ --algorithm ] arg   algorithms to use
  --algo_options arg       path to json file containing options for algorithms
  --algo_opt_string arg    json string (can be used instead of algo_options)
  --preset arg             path json preset file  !!!All other arguments will
                           be ignored!!!

Supported formats:
        jpeg

Supported algorithms:
        split_channels
```

## Explanation of arguments
### input_path
Input path to either a file or directory to process. The type of input (file/dir) is determined automatically.

### output_path
Output path of the same type as `input_path`. If `print_info` is also provided, it does not need to be passed.

### help
Produces help message (shown above).

### recurse
Can be only used when type of `input_path` is directory. Tells the program to also search all subdirectories.

### preset
Path to json preset file. Using this will ignore all other commands (except `help`).

File needs to have the following structure:
``` json
{
    "debug" : true, // defaults to false if ommited
    "recurse" : true, // defaults to false if ommited, ignored when input_path is file
    "print_info": true, // defaults to false if ommited
    "format" : "jpeg",
    "loading_options": { "compression" : "zlib" }, // does not need to contain all options, but needs to exist
    "saving_options": { "quality" : 50 }, // does not need to contain all options, but needs to exist
    "algorithms": [ "alg1", "alg2", ... ],
    "algo_options": { "alg1": { "fast": true } } // does not need to contain all algorithms, but needs to exist
}
```

### loading_options
Path to json file containing loading options. All options need to be falid for input format, ommited options will be set to default values.

File needs to have the following structure (same as `preset["loading_options"]`):
``` json
{
    "copmression": "zlib"
}
```
### loading_opt_string
Convenient option to pass the json string instead of file.

### format
Output format. The extension will be changed according to the final format.

If `input_path` is file, format can be ommited and will be deduces automatically from extension of `output_path`.

### saving_options
Same as `loading_options` with the difference that the options are used when saving.

### saving_opt_string
Convenient option to pass the json string instead of file.

### algorithm
Can be used more than once. Specifies algorithms to be used in a given order.

### algo_options
Path to json file containing algorithm options. All options need to be falid for input format, ommited options will be set to default values.

File needs to have the following structure (same as `preset["algo_options"]`):
``` json
{
    "algo1" : { "copmression": "zlib" },
    "algo2" : { "anti_aliasing": true }
}
```

### algo_opt_string
Convenient option to pass the json string instead of file.


## Supported formats and algorithms
All supported formats (for both, loading and saving) and algorithms are printed at the bottom of `help` output.

The list of supported options is not yet avaiable from CLI,
but can be found in the instalation directory in *formats* and *algorithms* in a form of json files.
