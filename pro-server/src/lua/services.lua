local services = {
    ["ifly"] = 2,
    ["cn.asr.rec"] = 2,
    ["en.asr.rec"] = 1,
    ["cn.tts"] = 1,
    ["en.tts"] = 1,
    ["dlgv2.0"] = 1,
    ["en.eval"] = 1,
    ["usc"]     = 1,
    ["mix"] = 1,
}

services.mixs = {
    mix = {"cn.asr.rec", "usc"},
}

return services
