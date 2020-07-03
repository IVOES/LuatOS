
/*
@module  i2c
@summary I2C操作
@version 1.0
@date    2020.03.30
*/
#include "luat_base.h"
#include "luat_log.h"
#include "luat_timer.h"
#include "luat_malloc.h"
#include "luat_i2c.h"

/*
i2c编号是否存在
@api i2c.exist(id)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@return int 存在就返回1,否则返回0
@usage
-- 检查i2c1是否存在
if i2c.exist(1) then
    log.info("存在 i2c1")
end
*/
static int l_i2c_exist(lua_State *L) {
    int re = luat_i2c_exist(luaL_checkinteger(L, 1));
    lua_pushinteger(L, re);
    return 1;
}

/*
i2c初始化
@api i2c.setup(id)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@return int 成功就返回1,否则返回0
@usage
-- 初始化i2c1
if i2c.setup(1) then
    log.info("存在 i2c1")
else
    i2c.close(1) -- 关掉
end
*/
static int l_i2c_setup(lua_State *L) {
    int re = luat_i2c_setup(luaL_checkinteger(L, 1), luaL_optinteger(L, 2, 0), luaL_optinteger(L, 3, 0));
    lua_pushinteger(L, re == 0 ? 1 : 0);
    return 1;
}

/*
i2c发送数据
@api i2c.send(id, addr, data)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@int I2C子设备的地址, 7位地址
@string 待发送的数据
@return nil 无返回值
@usage
-- 往i2c1发送2个字节的数据
i2c.send(1, 0x5C, string.char(0x0F, 0x2F))
*/
static int l_i2c_send(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    int addr = luaL_checkinteger(L, 2);
    size_t len;
    if (lua_isstring(L, 3)) {
        const char* buff = luaL_checklstring(L, 3, &len);
        luat_i2c_send(id, addr, (char*)buff, len);
    }
    else if (lua_isinteger(L, 3)) {
        len = lua_gettop(L) - 2;
        char buff[len+1];
        for (size_t i = 0; i < len; i++)
        {
            buff[i] = lua_tointeger(L, 3+i);
        }
        luat_i2c_send(id, addr, buff, len);
    }
    return 0;
}

/*
i2c接收数据
@api i2c.recv(id, addr, len)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@int I2C子设备的地址, 7位地址
@int 手机数据的长度
@return string 收到的数据
@usage
-- 从i2c1读取2个字节的数据
local data = i2c.recv(1, 0x5C, 2)
*/
static int l_i2c_recv(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    int addr = luaL_checkinteger(L, 2);
    int len = luaL_checkinteger(L, 3);
    char buf[len];
    luat_i2c_recv(id, addr, &buf[0], len);
    lua_pushlstring(L, buf, len);
    return 1;
}

/*
i2c接收数据
@api i2c.writeReg(id, addr, reg, data)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@int I2C子设备的地址, 7位地址
@int 寄存器地址
@string 待发送的数据
@return string 收到的数据
@usage
-- 从i2c1的地址为0x5C的设备的寄存器0x01写入2个字节的数据
i2c.writeReg(1, 0x5C, 0x01, string.char(0x00, 0xF2))
*/
static int l_i2c_write_reg(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    int addr = luaL_checkinteger(L, 2);
    int reg = luaL_checkinteger(L, 3);
    uint16_t value = luaL_checkinteger(L, 4);
    // size_t len;
    // if (lua_isstring(L, 4)) {
    //     const char* buff = luaL_checklstring(L, 4, &len);
    //     luat_i2c_write_reg(id, addr, reg, (char*)buff, len);
    // }
    // else if (lua_isinteger(L, 4)) {
    //     len = lua_gettop(L) - 2;
    //     char buff[len+1];
    //     for (size_t i = 0; i < len; i++)
    //     {
    //         buff[i] = lua_tointeger(L, 3+i);
    //     }
    //     luat_i2c_write_reg(id, addr, reg, buff, len);
    // }
    luat_i2c_write_reg(id, addr, reg, value);
    return 0;
}

static int l_i2c_read_reg(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    int addr = luaL_checkinteger(L, 2);
    int reg = luaL_checkinteger(L, 3);
    uint16_t value = 0;
    // size_t len;
    // if (lua_isstring(L, 4)) {
    //     const char* buff = luaL_checklstring(L, 4, &len);
    //     luat_i2c_write_reg(id, addr, reg, (char*)buff, len);
    // }
    // else if (lua_isinteger(L, 4)) {
    //     len = lua_gettop(L) - 2;
    //     char buff[len+1];
    //     for (size_t i = 0; i < len; i++)
    //     {
    //         buff[i] = lua_tointeger(L, 3+i);
    //     }
    //     luat_i2c_write_reg(id, addr, reg, buff, len);
    // }
    if (luat_i2c_read_reg(id, addr, reg, &value) == 0) {
        //
    }
    lua_pushinteger(L, value); 
    return 1;
}

/*
关闭i2c设备
@api i2c.close(id)
@int 设备id, 例如i2c1的id为1, i2c2的id为2
@return nil 无返回值
@usage
-- 关闭i2c1
i2c.close(1)
*/
static int l_i2c_close(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    luat_ic2_close(id);
    return 0;
}

#include "rotable.h"
static const rotable_Reg reg_i2c[] =
{
    { "exist", l_i2c_exist, 0},
    { "setup", l_i2c_setup, 0},
    { "send", l_i2c_send, 0},
    { "recv", l_i2c_recv, 0},
    { "writeReg", l_i2c_write_reg, 0},
    { "readReg", l_i2c_read_reg, 0},
    { "close", l_i2c_close, 0},
    { "FAST",  NULL, 1},
    { "SLOW",  NULL, 0},
	{ NULL, NULL, 0}
};

LUAMOD_API int luaopen_i2c( lua_State *L ) {
    rotable_newlib(L, reg_i2c);
    return 1;
}
