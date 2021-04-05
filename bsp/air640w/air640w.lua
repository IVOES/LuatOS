--[[
    This file is encode by "GBK"
]]
local sys = require "sys"

log.info("base", "LuatOS@Air640w ˢ������")

if rtos.bsp() ~= "win32" then
    log.info("base", "��ǰ��֧��win32����")
    os.exit()
end

local self_conf = {
    basic = {
        COM_PORT = "COM20",
        USER_PATH = "user\\",
        LIB_PATH = "lib\\",
        MAIN_LUA_DEBUG = "false",
        LUA_DEBUG = "false"
    }
}

local function trim5(s)
    return s:match'^%s*(.*%S)' or ''
end

local function read_local_ini()
    local f = io.open("/local.ini")
    if f then
        local key = "basic"
        for line in f:lines() do
            line = trim5(line)
            if #line > 0 then
                if line:byte() == '[' and line:byte(line:len()) then
                    key = line:sub(2, line:len() - 1)
                    if key == "air640w" then key = "basic" end
                    if self_conf[key] == nil then
                        self_conf[key] = {}
                    end
                elseif line:find("=") then
                    local skey = line:sub(1, line:find("=") - 1)
                    local val = line:sub(line:find("=") + 1)
                    if skey and val then
                        self_conf[key][trim5(skey)] = trim5(val)
                    end
                end
            end
        end
        f:close()
    end
    log.info("config", json.encode(self_conf))
end
read_local_ini()

local cmds = {}
cmds["help"] = function(arg)
    log.info("help", "==============================")
    log.info("help", "lfs      ����ļ�ϵͳ")
    log.info("help", "dlrom    ���صײ�̼�")
    log.info("help", "dlfs     �����ļ�ϵͳ")
    log.info("help", "dlfull   ���صײ�̼����ļ�ϵͳ")
    log.info("help", "==============================")
end

sys.taskInit(function()
    for _, arg in ipairs(win32.args()) do
        if cmds[arg] then
            cmds[arg]()
        elseif cmds["-" .. arg] then
            cmds[arg]()
        elseif arg:byte() == '-' and arg:find("=") then
            local skey = arg:sub(2, arg:find("=") - 1)
            local val = arg:sub(arg:find("=") + 1)
            if skey and val then
                self_conf["basic"][trim5(skey)] = trim5(val)
            end
        end
    end
    os.exit(0)
end)


sys.run()