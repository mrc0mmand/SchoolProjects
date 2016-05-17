#!/usr/bin/php
<?php
#CHA:xsumsa01

$options = array(
    "input"     => "./",
    "itype"     => "dir",
    "output"    => "php://stdout",
    //"indent"    => 0,
    "noinline"  => false,
    "maxpar"    => -1,
    "nodups"    => false,
    "nowhspace" => false,
    "sname"     => "",
);

$input_data = array();

/**
 * @brief Constants for error codes
 */
abstract class EC {
    const E_OK = 0;         /**< No error */
    const E_PARAM = 1;      /**< Error in input parameters */
    const E_INPUT = 2;      /**< Error in input file */
    const E_OUTPUT = 3;     /**< Error in output file */
    const E_INPUTFMT = 4;   /**< Error in input file format */
    const E_GENERAL = 100;  /**< General error */
}

/**
 * @brief Constants for finite-state machine
 */
abstract class StrState {
    const Code = 0;         /**< Code block */
    const LineComment = 1;  /**< Line comment */
    const BlockComment = 2; /**< Block comment */
    const MacroDef = 3;     /**< Code definition */
    const InStr = 4;        /**< String literal */
    const Slash = 5;        /**< Slash */
}

/**
 * @brief Because getopt() in PHP doesn't provide a reasonable way how to
 *        detect invalid arguments, it needs a workaround or a framework. This
 *        function basically strips all options found by getopt() function
 *        from $argv, so all remaining arguments in $argv can be considered
 *        as invalid ones.
 * @details This code was shamelessly copied from PHP docs:
 *          https://secure.php.net/manual/en/function.getopt.php#100573
 *
 * @param &$opts Array of parsed options from getopt() function
 */
function filter_opts(&$opts) {
    global $argv;
    global $options;
    $pruneargv = array();

    foreach($opts as $option => $value) {
        foreach($argv as $key => $chunk) {
            $regex = '/^'. (isset($option[1]) ? '--' : '-') . $option . '/';
            if($chunk == $value && $argv[$key-1][0] == '-'
               || preg_match($regex, $chunk)) {
                array_push($pruneargv, $key);
            }
        }
    }

    while($key = array_pop($pruneargv)) unset($argv[$key]);
    $options["sname"] = $argv[0];
    unset($argv[0]);
}

/**
 * @brief Check parsed options from command line and call appropriate
 *        functions.
 *
 * @param &$opts Array of parsed options from getopt() function
 */
function checks_opts(&$opts) {
    if(!is_array($opts)) {
        fwrite(STDERR, "Error: An error occured while parsing parameters\n");
        exit(EC::E_PARAM);
    }

    global $options;
    global $argv;
    filter_opts($opts);

    if(!empty($argv)) {
        fwrite(STDERR, "Error: Invalid arguments, see --help\n");
        exit(EC::E_PARAM);
    }

    foreach($opts as $key => $value) {
        if(is_array($value)) {
            fwrite(STDERR, "Error: Multiple --$key options specified\n");
            exit(EC::E_PARAM);
        }

        switch($key) {
        case "help":
            if(count($opts) > 1) {
                fwrite(STDERR, "Error: Invalid combination of parameters\n");
                exit(EC::E_PARAM);
            }

            echo "Usage: " . $options["sname"] . " [optional arguments]\n"
               . "\n"
               . "Arguments:\n"
               . "\t--help\t\t\t\tShows this help message\n"
               . "\t--input=fileordir\t\tInput file or directory\n"
               . "\t--output=filename\t\tOutput file\n"
               . "\t--pretty-xml[=n]\t\tWhen specified, each line of final XML"
                   . " will be appropriatelly indented\n\t\t\t\t\t(default 4"
                   . " spaces, can be overriden by specifying n as number of"
                   . " spaces\n"
               . "\t--no-inline\t\t\tSkips definitions/declarations of inline"
                   . " functions\n"
               . "\t--max-par=n\t\t\tWhen specified, only functions up to n"
                   . " parameters are parsed\n"
               . "\t--no-duplicates\t\t\tSkips multiple definitions/declarations"
                   . " of functions with the same name\n"
               . "\t--remove-whitespace\t\tRemoves unnecessary whitespaces\n";

            exit(EC::E_OK);
            break;
        case "input":
            $options["input"] = $value;
            break;
        case "output":
            $options["output"] = $value;
            break;
        case "pretty-xml":
            if($value !== false) {
                if(ctype_digit($value)) {
                    $options["indent"] = ($value >= 0 ? $value : 4);
                } else {
                    fwrite(STDERR, "Error: Invalid value for --$key ($value)\n");
                    exit(EC::E_PARAM);
                }
            } else {
                $options["indent"] = 4;
            }

            break;
        case "no-inline":
            $options["noinline"] = true;
            break;
        case "max-par":
            if(ctype_digit($value)) {
                $options["maxpar"] = $value;
            } else {
                fwrite(STDERR, "Error: Invalid value for --$key ($value)\n");
                exit(EC::E_PARAM);
            }

            break;
        case "no-duplicates":
            $options["nodups"] = true;
            break;
        case "remove-whitespace":
            $options["nowhspace"] = true;
            break;
        }
    }
}

/**
 * @brief Check if a function with given name is already in given array.
 *
 * @param &$a Array with functions
 * @param $fnc Function name
 *
 * @return True if function is found in the array, false otherwise.
 */
function is_fnc_in_array(&$a, $fnc) {
    foreach($a as $f) {
        if($f["name"] === $fnc)
            return true;
    }

    return false;
}

/**
 * @brief Simple finite-state machine, which strips all macro defintions,
 *        line/block comments and string literals from file content.
 *
 * @param &$c File content
 *
 * @return File content without comments, macros and string literals.
 */
function process_content(&$c) {
    $state = StrState::Code;
    $len = strlen($c);
    $str = "";

    for($i = 0; $i < $len; $i++) {
        if($state == StrState::Slash) {
            if($c[$i] == "/") {
                while(isset($c[++$i]) && $c[$i] != "\n");
            } else if($c[$i] == "*") {
                while((++$i + 1) < $len) {
                    if($c[$i] == '*' && $c[$i + 1] == "/") break; 
                }
                $i += 2;
            } else {
                $str .= "/";
            }

            $state = StrState::Code;
        } else if($c[$i] == "/") {
            $state = StrState::Slash;
            continue;
        } else if($c[$i] == "#") {
            if(substr_compare($c, "#define", $i)) {
                while(isset($c[++$i])) {
                    if($state == StrState::Slash && !ctype_space($c[$i])) {
                        $state = StrState::Code;
                    } else if($c[$i] == "\\") {
                        $state = StrState::Slash;
                    } else if($c[$i] == "\n") {
                        if($state == StrState::Slash) {
                            continue;
                        } else {
                            break;
                        }
                    }
                }

                $state = StrState::Code;
            }
        } else if($c[$i] == "\"") {
            while(isset($c[++$i]) && $c[$i] != "\"");
        }

        if(isset($c[$i])) {
            $str .= $c[$i];
        }
    }

    return $str;
}

/**
 * @brief Parse function definitions from given file.
 * @details Function uses several regular expressions to split each function
 *          definition into array, where each function has its name, data type,
 *          variable-length argument list information and a subarray of its
 *          arguments. Final results are saved into global array $input_data.
 *
 * @param $dir File directory
 * @param $file File name
 */
function parse_file($dir, $file) {
    if($dir !== "" && !preg_match("/\/$/", $dir)) {
        $dir = "$dir/";
    }

    global $input_data;
    global $options;
    $path = "$dir$file";
    $in = fopen($path, "r");

    if($in === false) {
        fwrite(STDERR, "Error: Unable to open file $path\n");
        exit(EC::E_INPUT);
    }

    $content = @fread($in, filesize($path));
    $content = process_content($content);

    $fnc_pattern = "/(?'dtype'[[:alnum:]_][[:alnum:]_\s\*]+?(?!\s*\())"
                 . "(?'name'[[:alnum:]_]+)\s*\(\s*(?'args'[^)]*)\s*\);?/";
    $arg_pattern = "/(((?'dtype'[[:alnum:]_\s\*]+)\s*?(?'name'(?<=(\s|\*))"
                 . "[[:alnum:]_]+))|\s*(?'va'[.]{3})\s*)\,?/";

    preg_match_all($fnc_pattern, $content, $functions, PREG_SET_ORDER);
    foreach($functions as $function) {
        $fnc_args = array();
        $va_args = false;
        $fnc_skip = false;
        $arg_cnt = 0;

        if($options["noinline"] && preg_match("/\binline\b/", $function["dtype"])) {
            continue;
        }

        preg_match_all($arg_pattern, $function["args"], $args, PREG_SET_ORDER);
        foreach($args as $arg) {
            if($options["nowhspace"]) {
                $arg["dtype"] = preg_replace("/\s+/", " ", $arg["dtype"]);
                $arg["dtype"] = preg_replace("/(.+?)\s+(\*)\s*/", "\\1\\2", $arg["dtype"]);
            }

            if(array_key_exists("va", $arg)) {
                $va_args = true;
            } else {
                $arg_cnt++;
                array_push($fnc_args, array("dtype" => trim($arg["dtype"]),
                                            "name" => trim($arg["name"])));
            }

            if($options["maxpar"] !== -1 && $arg_cnt > (int)$options["maxpar"]) {
                $fnc_skip = true;
                break;
            }
       }

        if($fnc_skip) {
            continue;
        }

        $rel_path = $options["input"];

        if($options["itype"] !== "file") {
            $split_pattern = "^" . $options["input"] . "(?'dir'.+)$";
            $split_pattern = preg_replace("/\//", "\/", $split_pattern);
            preg_match("/$split_pattern/", $path, $m);
            $rel_path = $m['dir'];
        }

        $function["dtype"] = trim($function["dtype"]);
        $function["name"] = trim($function["name"]);

        if($options["nowhspace"]) {
            $function["dtype"] = preg_replace("/\s+/", " ",
                                              $function["dtype"]);
            $function["dtype"] = preg_replace("/(.+?)\s+(\*)/", "\\1\\2",
                                              $function["dtype"]);
        }

        if($options["nodups"] && is_fnc_in_array($input_data, $function["name"])) {
            continue;
        }

        $input_data[] = array("dtype" => $function["dtype"],
                              "args" => $fnc_args,
                              "file" => $rel_path,
                              "name" => $function["name"],
                              "vaargs" => $va_args);
    }

    fclose($in);
}

/**
 * @brief Scan given directory recursively and call parse_file function on
 *        each C header (*.h) file.
 *
 * @param $dir Directory to scan
 */
function scan_dir($dir) {
    $cdir = scandir($dir);

    if($cdir === false) {
        fwrite(STDERR, "Error: Unable to open directory $dir\n");
        exit(EC::E_INPUT);
    }

    foreach($cdir as $key => $value) {
        if(!in_array($value, array(".", ".."))) {
            if(is_dir($dir . DIRECTORY_SEPARATOR . $value)) {
                if(preg_match("/\\" . DIRECTORY_SEPARATOR . "$/", $dir)) {
                    $d = $dir . $value;
                } else {
                    $d = $dir . DIRECTORY_SEPARATOR . $value;
                }
                $r = scan_dir($d);
            } else {
                if(preg_match("/\.h$/", $value)) {
                    parse_file($dir, $value);
                }
            }
        }
    }
}

/**
 * @brief Check parsed command line options saved in global $options array and
 *        call appropriate function based on input type - scan_dir if input
 *        is a directory, parse file in case of file.
 */
function process_input() {
    global $options;

    switch(filetype($options["input"])) {
    case "dir":
        if(!preg_match("/\/$/", $options["input"])) {
            $options["input"] = $options["input"] . "/";
        }

        scan_dir($options["input"]);
        break;
    case "file":
        $options["itype"] = "file";
        parse_file("", $options["input"]);
        break;
    default:
        fwrite(STDERR, "Error: Invalid input file\n");
        exit(EC::E_INPUT);
    }
}

/**
 * @brief Generate final XML file based on information saved in global
 *        $input_data array.
 */
function generate_XML() {
    global $options;
    global $input_data;
    $xmlWriter = new XMLWriter();
    if(!$xmlWriter->openUri($options["output"])) {
        fwrite(STDERR, "Error: Unable to open output file\n");
        exit(EC::E_OUTPUT);
    }

    if(array_key_exists("indent", $options)) {
        $xmlWriter->setIndent(true);
        $xmlWriter->setIndentString(str_repeat(" ", $options["indent"]));
    }

    if($xmlWriter) {
        /* Due to specific conditions from the assignment, I can't use
         * startDocument and endDocument methods
         */
        //$xmlWriter->startDocument("1.0", "UTF-8");
        $xmlWriter->writeRaw("<?xml version=\"1.0\" encoding=\"UTF-8\"?>" . 
                             (array_key_exists("indent", $options) ? "\n" : ""));

        if(!empty($input_data)) {
            $xmlWriter->startElement("functions");
            $xmlWriter->writeAttribute("dir", ($options["itype"] === "file") ?
                                               "" : $options["input"]);

            foreach($input_data as $function) {
                $idx = 0;

                $xmlWriter->startElement("function");
                $xmlWriter->writeAttribute("file", $function["file"]);
                $xmlWriter->writeAttribute("name", $function["name"]);
                $xmlWriter->writeAttribute("varargs", $function["vaargs"] ?
                                                      "yes" : "no");
                $xmlWriter->writeAttribute("rettype", $function["dtype"]);

                foreach($function["args"] as $arg) {
                    $xmlWriter->startElement("param");
                    $xmlWriter->writeAttribute("number", ++$idx);
                    $xmlWriter->writeAttribute("type", $arg["dtype"]);
                    $xmlWriter->endElement();
                }

                $xmlWriter->endElement();
            }

            $xmlWriter->endElement();
        }

        //$xmlWriter->endDocument();
        $xmlWriter->flush();
    } else {
        fwrite(STDERR, "Error: Couldn't create XMLWrite instance\n");
        exit(EC::E_GENERAL);
    }
}

$longopts = array (
    "help",
    "input:",
    "output:",
    "pretty-xml::",
    "no-inline",
    "max-par:",
    "no-duplicates",
    "remove-whitespace"
);

$opts = getopt("", $longopts);
checks_opts($opts);
process_input();
generate_XML();

return EC::E_OK;
?>
