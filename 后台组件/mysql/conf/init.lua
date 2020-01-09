--[
--  安装lua 5.3见：https://www.lua.org/start.html
--      sudo apt-get install libreadline-dev
--      curl -R -O http://www.lua.org/ftp/lua-5.3.5.tar.gz
--      tar zxf lua-5.3.5.tar.gz
--      cd lua-5.3.5
--      make linux test
--      sudo make install
--
--  安装luarocks：https://github.com/luarocks/luarocks
--      安装步骤见：https://github.com/luarocks/luarocks/wiki/Installation-instructions-for-Unix
--
--  安装 luasql-mysql: luarocks install luasql-mysql MYSQL_INCDIR=/usr/include/mysql
--      其中MYSQL_INCDIR指的是c 的mysqlclient安装的位置
--]

mysql = require "luasql.mysql"
math  = require "math"
uuid  = require "uuid"

local function generateUUID()
    local template ="xxxxxxxxxxxxxx"
    d = io.open("/dev/urandom", "r"):read(4)
    math.randomseed(os.time() + d:byte(1) + (d:byte(2) * 256) + (d:byte(3) * 65536) + (d:byte(4) * 4294967296))
    return string.gsub(template, "x", function (c)
        local v = (c == "x") and math.random(0, 0xf) or math.random(8, 0xb)
        return string.format("%x", v)
      end)
end

-- generateUUID = uuid

local env   = mysql:mysql()
local conn  = env:connect("ClondVoice", "root", "123456", "127.0.0.1", 3306)
if conn == nil then
    print("connect mysql failed\n")
    return
end

conn:execute("set names UTF8")

-- 构造数据库的数据

-- 构造user数据
-- [
--  +--------------+--------------+------+-----+---------+----------------+
--  | Field        | Type         | Null | Key | Default | Extra          |
--  +--------------+--------------+------+-----+---------+----------------+
--  | uid          | int(11)      | NO   | PRI | NULL    | auto_increment |
--  | name         | varchar(128) | NO   |     | NULL    |                |
--  | account      | varchar(32)  | NO   |     | NULL    |                |
--  | password     | varchar(32)  | NO   |     | NULL    |                |
--  | type         | smallint(6)  | NO   |     | NULL    |                |
--  | identifyCard | varchar(32)  | NO   |     | NULL    |                |
--  | registerDate | bigint(20)   | NO   |     | NULL    |                |
--  | email        | varchar(32)  | NO   |     | NULL    |                |
--  +--------------+--------------+------+-----+---------+----------------+
-- ]

print(os.date())


--math.randomseed(os.time())
for i = 1, 10000, 1 do
    account      = generateUUID() --math.random()
    name         = "name" .. account
    password     = "passwd" .. account
    utype        = 0
    identifyCard = "id" .. account
    registerDate = os.time()
    email        = "lizhiyong" .. account .. "@gmail.com"

    sql = string.format("insert into t_user  (name, account, password, type, identifyCard, registerDate, email) values('%s', '%s', '%s', %d, '%s', %d, '%s');", name, account, password, utype, identifyCard, registerDate, email)
    ret, err = conn:execute(sql)
    if (ret == nil) then
	print(sql)
        print(err)
	return
    end
end


-- 构造t_dev表
--[
-- +--------------+-------------+------+-----+---------+----------------+
-- | Field        | Type        | Null | Key | Default | Extra          |
-- +--------------+-------------+------+-----+---------+----------------+
-- | id           | int(11)     | NO   | PRI | NULL    | auto_increment |
-- | sn           | varchar(64) | NO   | UNI | NULL    |                |
-- | sw_version   | varchar(20) | YES  |     | NULL    |                |
-- | fw_version   | varchar(20) | YES  |     | NULL    |                |
-- | product_type | varchar(20) | YES  |     | NULL    |                |
-- | node_server  | varchar(40) | YES  |     | NULL    |                |
-- | app_secret   | varchar(32) | YES  |     | NULL    |                |
-- | ip           | varchar(40) | YES  |     | NULL    |                |
-- | app_key      | varchar(32) | YES  |     | NULL    |                |
-- | frd          | bigint(64)  | YES  |     | NULL    |                |
-- | lrd          | bigint(64)  | YES  |     | NULL    |                |
-- | volume       | int(11)     | YES  |     | 80      |                |
-- +--------------+-------------+------+-----+---------+----------------+
--]

--[[
for i = 1, 10000, 1 do
    sn           = generateUUID()
    sw_version   = "0.8.1"
    fw_version   = "V100.0019.01"
    product_type = "0"
    node_server  = "192.168.0.1"
    app_secret   = generateUUID()
    ip           = "192.168.100.1"
    app_key      = sn --generateUUID()
    frd          = os.time()
    lrd          = os.time()
    volume        = 75

    sql = string.format("insert into t_dev (sn, sw_version, fw_version, product_type, node_server, app_secret, ip, app_key, frd, lrd, volume) values ('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', %d, %d, %d);", sn, sw_version, fw_version, product_type, node_server, app_secret, ip, app_key, frd, lrd, volume)
    ret, err = conn:execute(sql)
    if ret == nil then
        print(sql)
	print(err)
	return;
    end
end

--]]



--[[
-- 构造用户的设备表：
-- +----------+-------------+------+-----+---------+-------+
-- | Field    | Type        | Null | Key | Default | Extra |
-- +----------+-------------+------+-----+---------+-------+
-- | devId    | int(11)     | NO   | PRI | NULL    |       |
-- | uid      | int(11)     | NO   | MUL | NULL    |       |
-- | name     | varchar(32) | YES  |     | NULL    |       |
-- | bindTime | bigint(64)  | NO   |     | NULL    |       |
-- +----------+-------------+------+-----+---------+-------+
--]]

--[[
for i = 1, 10000, 1 do
    for j = 1, 2, 1 do
        devId    = i*2 + j
	uid      = i
	name     = i
	bindTime = os.time()
	sql = string.format("insert into t_user_dev values(%d, %d, '%s', %d)", devId, uid, name, bindTime)
	ret , err = conn:execute(sql)
	if ret == nil then
	    print(sql)
	    print(err)
	end
    end
end
--]]

conn:close()
env:close()

