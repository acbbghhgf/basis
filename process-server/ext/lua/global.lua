local global = {}

global.iris = require "iris"
global.ebnf = require "ebnf"
global.hotword = require "hotword"

global.test = false


global.remote_console = {
   listen = "0.0.0.0",
   port = 8081,
   backlog = 5,
}


global.err_code = {
   engine_not_found = 60001,
   engine_create_failed = 60002,
   engine_pipe_broken = 60003,
   engine_response_timeout = 60004,
   engine_file_write = 60005,
   read_input_timeout = 60006,
   engine_unknown = 60099,
   engine_ebnf_server_refused = 61001,
   engine_ebnf_dl_timeout = 61002,
   engine_ebnf_dl_failed = 61003,
   engine_ebnf_not_found = 61004,
   engine_ebnf_dl_unknown = 61099,
}


global.services = {
   "cn.tts",
   "en.tts",
   "cn.asr.rec",
   "ifly",
   "en.eval",
   "usc",
}

return global
