local server_ip = "112.125.89.8"    --如果用TCP服务器，目前需要在用极致功耗模式时先断开服务器
local server_port = 41101 --换成自己的
local period = 1 * 60 * 1000 --一分钟唤醒一次

local reason, slp_state = pm.lastReson()
log.info("wakeup state", pm.lastReson())
local libnet = require "libnet"

local d1Name = "D1_TASK"
local function netCB(msg)
	log.info("未处理消息", msg[1], msg[2], msg[3], msg[4])
end

local function testTask(ip, port)
    local txData
    if reason == 0 then
        txData = "normal wakeup"
    elseif reason == 1 then
        txData = "timer wakeup"
    elseif reason == 2 then
        txData = "pad wakeup"
    elseif reason == 3 then
        txData = "uart1 wakeup"
    end

    gpio.close(32)

	local netc, needBreak
	local result, param, is_err
	netc = socket.create(nil, d1Name)
	socket.debug(netc, false)
	socket.config(netc) -- demo用TCP服务器，目前需要在用极致功耗模式时先断开服务器
    local retry = 0
	while retry < 3 do
		log.info(rtos.meminfo("sys"))
		result = libnet.waitLink(d1Name, 0, netc)
		result = libnet.connect(d1Name, 5000, netc, ip, port)
		if result then
			log.info("服务器连上了")
            result, param = libnet.tx(d1Name, 15000, netc, txData)
			if not result then
				log.info("服务器断开了", result, param)
				break
			else
                needBreak = true
            end
		else
            log.info("服务器连接失败")
        end
		libnet.close(d1Name, 5000, netc)
        retry = retry + 1
        if needBreak then
            break
        end
	end

    uart.setup(1, 9600)
    gpio.setup(23,nil)
    gpio.close(35)
    gpio.setup(32, function()
        log.info("gpio")
    end, gpio.PULLUP, gpio.FALLING)
    pm.dtimerStart(3, period)
    pm.power(pm.WORK_MODE,3)
    log.info(rtos.meminfo("sys"))
    sys.wait(15000)
    log.info("进入极致功耗模式失败，尝试重启")
    rtos.reboot()
end
sysplus.taskInitEx(testTask, d1Name, netCB, server_ip, server_port)