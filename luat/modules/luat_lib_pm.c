
/*
@module  pm
@summary 电源管理
@version 1.0
@date    2020.07.02
@demo pm
@tag LUAT_USE_PM
*/
#include "lua.h"
#include "lauxlib.h"
#include "luat_base.h"
#include "luat_pm.h"
#include "luat_msgbus.h"

#define LUAT_LOG_TAG "pm"
#include "luat_log.h"

// static int lua_event_cb = 0;

/**
请求进入指定的休眠模式
@api pm.request(mode)
@int 休眠模式,例如pm.IDLE/LIGHT/DEEP/HIB.
@return boolean 处理结果,即使返回成功,也不一定会进入, 也不会马上进入
@usage
-- 请求进入休眠模式
--[[
IDLE   正常运行,就是无休眠
LIGHT  轻休眠, CPU停止, RAM保持, 外设保持, 可中断唤醒. 部分型号支持从休眠处继续运行
DEEP   深休眠, CPU停止, RAM掉电, 仅特殊引脚保持的休眠前的电平, 大部分管脚不能唤醒设备.
HIB    彻底休眠, CPU停止, RAM掉电, 仅复位/特殊唤醒管脚可唤醒设备.
]]

pm.request(pm.HIB)
 */
static int l_pm_request(lua_State *L) {
    int mode = luaL_checkinteger(L, 1);
    if (luat_pm_request(mode) == 0)
        lua_pushboolean(L, 1);
    else
        lua_pushboolean(L, 0);
    return 1;
}

// static int l_pm_release(lua_State *L) {
//     int mode = luaL_checkinteger(L, 1);
//     if (luat_pm_release(mode) == 0)
//         lua_pushboolean(L, 1);
//     else
//         lua_pushboolean(L, 0);
//     return 1;
// }

/**
启动底层定时器,在休眠模式下依然生效. 只触发一次
@api pm.dtimerStart(id, timeout)
@int 定时器id,通常是0-3
@int 定时时长,单位毫秒
@return boolean 处理结果
@usage
-- 添加底层定时器
pm.dtimerStart(0, 300 * 1000) -- 5分钟后唤醒
 */
static int l_pm_dtimer_start(lua_State *L) {
    int dtimer_id = luaL_checkinteger(L, 1);
    int timeout = luaL_checkinteger(L, 2);
    if (luat_pm_dtimer_start(dtimer_id, timeout)) {
        lua_pushboolean(L, 0);
    }
    else {
        lua_pushboolean(L, 1);
    }
    return 1;
}

/**
关闭底层定时器
@api pm.dtimerStop(id)
@int 定时器id
@usage
-- 关闭底层定时器
pm.dtimerStop(0) -- 关闭id=0的底层定时器
 */
static int l_pm_dtimer_stop(lua_State *L) {
    int dtimer_id = luaL_checkinteger(L, 1);
    luat_pm_dtimer_stop(dtimer_id);
    return 0;
}

/**
检查底层定时器是不是在运行
@api pm.dtimerCheck(id)
@int 定时器id
@return boolean 处理结果,true还在运行，false不在运行
@usage
-- 检查底层定时器是不是在运行
pm.dtimerCheck(0) -- 检查id=0的底层定时器
 */
static int l_pm_dtimer_check(lua_State *L) {
    int dtimer_id = luaL_checkinteger(L, 1);
    if (luat_pm_dtimer_check(dtimer_id))
    {
    	lua_pushboolean(L, 1);
    }
    else
    {
    	lua_pushboolean(L, 0);
    }
    return 1;
}

static int l_pm_dtimer_list(lua_State *L) {
    size_t c = 0;
    size_t dlist[24];

    luat_pm_dtimer_list(&c, dlist);

    lua_createtable(L, c, 0);
    for (size_t i = 0; i < c; i++)
    {
        if (dlist[i] > 0) {
            lua_pushinteger(L, dlist[i]);
            lua_seti(L, -3, i+1);
        }
    }

    return 1;
}

static int l_pm_dtimer_wakeup_id(lua_State *L) {
    int dtimer_id = 0xFF;

    luat_pm_dtimer_wakeup_id(&dtimer_id);

    if (dtimer_id != 0xFF) {
        lua_pushinteger(L, dtimer_id);
    }
    else {
        lua_pushinteger(L, -1);
    }
    return 1;
}

// static int l_pm_on(lua_State *L) {
//     if (lua_isfunction(L, 1)) {
//         if (lua_event_cb != 0) {
//             luaL_unref(L, LUA_REGISTRYINDEX, lua_event_cb);
//         }
//         lua_event_cb = luaL_ref(L, LUA_REGISTRYINDEX);
//     }
//     else if (lua_event_cb != 0) {
//         luaL_unref(L, LUA_REGISTRYINDEX, lua_event_cb);
//     }
//     return 0;
// }

/**
开机原因,用于判断是从休眠模块开机,还是电源/复位开机
@api pm.lastReson()
@return int 0-上电/复位开机, 1-RTC开机, 2-WakeupIn/Pad/IO开机, 3-Wakeup/RTC开机
@return int 0-普通开机(上电/复位),3-深睡眠开机,4-休眠开机，3和4说明程序已经复位了
@return int 复位开机详细原因：0-powerkey或者上电开机 1-充电或者AT指令下载完成后开机 2-闹钟开机 3-软件重启 4-未知原因 5-RESET键 6-异常重启 7-工具控制重启 8-内部看门狗重启 9-外部重启 10-充电开机
@usage
-- 是哪种方式开机呢
log.info("pm", "last power reson", pm.lastReson())
 */
static int l_pm_last_reson(lua_State *L) {
    int lastState = 0;
    int rtcOrPad = 0;
    luat_pm_last_state(&lastState, &rtcOrPad);
    lua_pushinteger(L, rtcOrPad);
    lua_pushinteger(L, lastState);
    lua_pushinteger(L, luat_pm_get_poweron_reason());
    return 3;
}

/**
强制进入指定的休眠模式，忽略某些外设的影响，比如USB
@api pm.force(mode)
@int 休眠模式
@return boolean 处理结果,若返回成功,大概率会马上进入该休眠模式
@usage
-- 请求进入休眠模式
pm.force(pm.HIB)
 */
static int l_pm_force(lua_State *L) {
    lua_pushinteger(L, luat_pm_force(luaL_checkinteger(L, 1)));
    return 1;
}

/**
检查休眠状态,仅air302适用.
@api pm.check()
@return boolean 处理结果,如果能顺利进入休眠,返回true,否则返回false
@return int 底层返回值,0代表能进入最底层休眠,其他值代表最低可休眠级别
@usage
-- 请求进入休眠模式,然后检查是否能真的休眠
pm.request(pm.HIB)
if pm.check() then
    log.info("pm", "it is ok to hib")
else
    pm.force(pm.HIB) -- 强制休眠
end
 */
static int l_pm_check(lua_State *L) {
    int ret = luat_pm_check();
    lua_pushboolean(L, luat_pm_check() == 0 ? 1 : 0);
    lua_pushinteger(L, ret);
    return 2;
}


#ifndef LUAT_COMPILER_NOWEAK
LUAT_WEAK int luat_pm_poweroff(void) {
    LLOGW("powerOff is not supported");
    return -1;
}
#else
extern int luat_pm_poweroff(void);
#endif

/**
关机
@api pm.shutdown()
@return nil 无返回值
@usage
-- 当前仅EC618系列(Air780E/Air600E/Air700E/Air780EG支持)
-- 需要2022-12-22之后编译的固件
pm.shutdown()
 */
static int l_pm_power_off(lua_State *L) {
    (void)L;
    luat_pm_poweroff();
    return 0;
}

/**
重启
@api pm.reboot()
@return nil          无返回值
-- 立即重启设备, 本函数的行为与rtos.reboot()完全一致,只是在pm库做个别名
pm.reboot()
 */
int l_rtos_reboot(lua_State *L);
int l_rtos_standby(lua_State *L);

/**
开启内部的电源控制，注意不是所有的平台都支持，可能部分平台支持部分选项，看硬件
@api pm.power(id, onoff)
@int 电源控制id,pm.USB pm.GPS pm.GPS_ANT之类
@boolean 开关true开，false关，默认关
@return boolean 处理结果true成功，false失败
@usage
pm.power(pm.USB, false) --关闭USB电源
 */
static int l_pm_power_ctrl(lua_State *L) {
	uint8_t onoff = 0;
    int id = luaL_checkinteger(L, 1);
    if (lua_isboolean(L, 2)) {
    	onoff = lua_toboolean(L, 2);

    }
    lua_pushboolean(L, !luat_pm_power_ctrl(id, onoff));
    return 1;
}

#include "rotable2.h"
static const rotable_Reg_t reg_pm[] =
{
    { "request" ,       ROREG_FUNC(l_pm_request )},
    // { "release" ,    ROREG_FUNC(   l_pm_release)},
    { "dtimerStart",    ROREG_FUNC(l_pm_dtimer_start)},
    { "dtimerStop" ,    ROREG_FUNC(l_pm_dtimer_stop)},
	{ "dtimerCheck" ,   ROREG_FUNC(l_pm_dtimer_check)},
    { "dtimerList",     ROREG_FUNC(l_pm_dtimer_list)},
    { "dtimerWkId",     ROREG_FUNC(l_pm_dtimer_wakeup_id)},
    //{ "on",           ROREG_FUNC(l_pm_on)},
    { "force",          ROREG_FUNC(l_pm_force)},
    { "check",          ROREG_FUNC(l_pm_check)},
    { "lastReson",      ROREG_FUNC(l_pm_last_reson)},
    { "shutdown",       ROREG_FUNC(l_pm_power_off)},
    { "reboot",         ROREG_FUNC(l_rtos_reboot)},
	{ "power",         ROREG_FUNC(l_pm_power_ctrl)},
    //@const NONE number 不休眠模式
    { "NONE",           ROREG_INT(LUAT_PM_SLEEP_MODE_NONE)},
    //@const IDLE number IDLE模式
    { "IDLE",           ROREG_INT(LUAT_PM_SLEEP_MODE_IDLE)},
    //@const LIGHT number LIGHT模式
    { "LIGHT",          ROREG_INT(LUAT_PM_SLEEP_MODE_LIGHT)},
    //@const DEEP number DEEP模式
    { "DEEP",           ROREG_INT(LUAT_PM_SLEEP_MODE_DEEP)},
    //@const HIB number HIB模式
    { "HIB",            ROREG_INT(LUAT_PM_SLEEP_MODE_STANDBY)},
    //@const USB number USB电源
    { "USB",            ROREG_INT(LUAT_PM_POWER_USB)},
    //@const GPS number GPS电源
    { "GPS",            ROREG_INT(LUAT_PM_POWER_GPS)},
    //@const GPS_ANT number GPS的天线电源，有源天线才需要
    { "GPS_ANT",        ROREG_INT(LUAT_PM_POWER_GPS_ANT)},

	{ NULL,             ROREG_INT(0) }
};

LUAMOD_API int luaopen_pm( lua_State *L ) {
    luat_newlib2(L, reg_pm);
    return 1;
}
