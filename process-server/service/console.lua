local zeus = require "qtk.zeus"
local srvx = require "qtk.zeus.srvx"


local function split_cmdline(cmdline)
   local split = {}
   for i in string.gmatch(cmdline, "%S+") do
      table.insert(split, i)
   end
   return split
end


local function console_main_loop()
   for cmdline in io.lines() do
      local split = split_cmdline(cmdline)
      local command = split[1]
      if command == "srvx" then
         pcall(srvx.newservice, select(2, table.unpack(split)))
      elseif cmdline ~= "" then
         pcall(zeus.newservice, cmdline)
      end
   end
end


zeus.start(function()
      zeus.fork(console_main_loop)
end)
