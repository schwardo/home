module("L_DenonReceiver1", package.seeall)

local socket = require("socket")

local SWP_SID = "urn:upnp-org:serviceId:SwitchPower1"
local SWP_STATUS = "Status"
local SWP_TARGET = "Target"
local SWP_SET_TARGET = "SetTarget"

local IPS_IID = "urn:micasaverde-com:serviceId:InputSelection1"
local VOS_VID = "urn:micasaverde-com:serviceId:Volume1"
local DEN_DID = "urn:denon-com:serviceId:Receiver1"

local DEVICETYPE_DENON_AVR_CONTROL = "urn:schemas-denon-com:device:receiverZone:1"
local DEVICEFILE_DENON_AVR_CONTROL = "D_DenonReceiverZone1.xml"

local MAX_VOLUME = 98

local sourceName = {}
local zoneNameTable = {}

local childDeviceZone = {}

local receiverDetails = {
    ["AVR-3808EUR"]={ zones={3}, inputs={}, names={} },
    ["AVR-2112USA"]={ zones={2}, inputs={}, names={} },
    ["default"]={ zones={3}, inputs={}, names={} }
}

local denonDevice

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
-- UTILITIES
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

------------------------------------------------------------
-- Used when running code not on Vera
function set_debug_vars()
    denonDevice = 'AVR-3808USA'
    childDeviceZone = { Z1 = 'main zone', Z2 = 'zone two', Z3 = 'zone three' }
end

------------------------------------------------------------
function log(text, level)
    luup.log("Denon AVR plugin: " .. text, (level or 1))
end

--------------------------------------------------------------------------------
function findZone(lul_device)
    log("Trying to find zone for device " .. lul_device)
    local DeviceId = tostring(luup.devices[lul_device].id)
    local ParentDevice = tostring(luup.devices[lul_device].device_num_parent)
    log("DeviceID=" .. DeviceId .. ", Parent="..ParentDevice)
    if ParentDevice == nil then
        zone = ""
    else
        zone = DeviceId
    end
    log("Zone: " .. zone)
    return zone
end

--------------------------------------------------------------------------------
function findChild(label)
    return childDeviceZone[label]
end

--------------------------------------------------------------------------------
-- make sure that variables have reasonable default values
local function check_var (var, default)
    if (var == '' or var == nil) then
        var = default
    end
    return var
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
-- OUTGOING COMMANDS
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
function setVolume(device, value)
    sendZoneCommand(device, 'MV', string.format("%02d", ((value>MAX_VOLUME) and MAX_VOLUME or value)), 'Volume set')
end

--------------------------------------------------------------------------------
function sendZoneCommand(device, prefix, command, commandNameString)
    local zone = findZone(device)
    log((commandNameString)  .. zone)
    -- If this is not the main zone we need to change the prefix to the zone string
    if (zone ~= "Z1") then
        prefix = zone
    end
    log("(sendZoneCommand)" .. prefix .. command)
    denon3800ReceiverSend(prefix .. command)
end

--------------------------------------------------------------------------------
function denon3800ReceiverSend(command)
    local cmd = command
    local startTime, endTime
    local dataSize = string.len(cmd)
    assert(dataSize <= 135)
    startTime = socket.gettime()
    luup.sleep(200)
    if (luup.io.is_connected == false) then
        log("(denon3800ReceiverSend) cannot send command " .. command .. " receiver not connected", 1)
        luup.set_failure(true)
        return false
    end
    if (luup.io.write(cmd, denonDevice) == false) then
        log("(denon3800ReceiverSend) cannot send command " .. command .. " communications error", 1)
        luup.set_failure(true)
        return false
    end
    endTime = socket.gettime()
    log("Request returned in " .. math.floor((endTime - startTime) * 1000) .. "ms")
    return true
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
-- STARTUP FUNCTIONS
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
local function initialParameters(denonDevice)
    luup.variable_set(SWP_SID,SWP_STATUS,"0",denonDevice)
    denon3800ReceiverSend("PW?")

    for k, v in pairs(childDeviceZone) do
        local child_id = k
        luup.variable_set(SWP_SID, SWP_STATUS, "0",denonDevice)
        luup.variable_set(IPS_IID, "Surround", "0" ,child_id)
        luup.variable_set(IPS_IID, "Input",    "0", child_id)
        luup.variable_set(IPS_IID, "Video",    "0", child_id)
        luup.variable_set(VOS_VID, "Mute",     "0", child_id)
        luup.variable_set(VOS_VID, "Volume",   "0", child_id) 
    end
    
    denon3800ReceiverSend("MS?")
    denon3800ReceiverSend("SI?")
    denon3800ReceiverSend("SV?")
    denon3800ReceiverSend("MU?")
    denon3800ReceiverSend("MV?")
end

--------------------------------------------------------------------------------
local function findChildren(ParentDevice)
    for k, v in pairs(luup.devices) do
        if (tostring(v.device_num_parent) == tostring(ParentDevice)) then
            childDeviceZone[v.id]=k
            log("(findChild) device number: " .. k .. " Zone: " .. v.id)
        end
    end
end

--------------------------------------------------------------------------------
function createChildren()
    log("(createChildren) Starting")
    child_devices = luup.chdev.start(denonDevice)
      for k, v in pairs(zoneNameTable) do
        local zoneNumber = k:sub(2,2)
        local zoneName = v:gsub ("(%s+)$", "")
        zoneName = check_var(zoneName, "Denon AVR Zone: " .. zoneNumber)
        luup.chdev.append(denonDevice,child_devices, "Z" .. zoneNumber, zoneName,
                          DEVICETYPE_DENON_AVR_CONTROL, DEVICEFILE_DENON_AVR_CONTROL, "", "", true)
        log("(createChildren) Zone number:" .. k .. " Zone name:" .. zoneName .. ".")
    end

    luup.chdev.sync(denonDevice, child_devices)

    -- setup lookup table from zone -> child
    findChildren(denonDevice)
end

--------------------------------------------------------------------------------
function receiverStartup(lul_device)

    local ipAddress, trash, ipPort = string.match(luup.devices[lul_device].ip, "^([%w%.%-]+)(:?(%d-))$")


    if (ipAddress and ipAddress ~= "") then
        ipPort = check_var (ipPort, "23")
        log(string.format ("(receiverStartup) ipAddress=%s, ipPort=%s", tostring (ipAddress), tostring (ipPort)))
        luup.io.open (lul_device, ipAddress, ipPort)
    else
        log("(receiverStartup) Running on Serial.")
    end
    denonDevice = lul_device

    --Returns custom source names, creates table but names are not currently used.
    deviceAvail = denon3800ReceiverSend("SSFUN ?")

    --get zone names
    local getZoneNames = 'http://' .. ipAddress .. '/ZONERENAME/r_zonerename.asp'
    log("(receiverStartup) force AVR to send zone names" .. getZoneNames) status, response = luup.inet.wget(getZoneNames)

    if (deviceAvail == false) then
        log("(receiverStartup) device not currently available. Plugin exiting")
        return false
    else
        --main zone initial parameters
        initialParameters(denonDevice)

        -- we want to make sure we have time for the child zones to be filled in the zoneNameTable
        luup.call_delay('add_children', 10, denonDevice)

        for k, v in pairs(zoneNameTable) do
            if(k ~= "R1") then
                local zoneNumber = k:sub(2,2)
                denon3800ReceiverSend("Z" .. zoneNumber .. "?")
            end
        end

        --Returns model.
        denon3800ReceiverSend("SYMO")
        return true
    end
end

--------------------------------------------------------------------------------
--------------------------------------------------------------------------------
-- HANDLE INCOMING DATA
--------------------------------------------------------------------------------
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
local function set_volume(dev, vol)
    if vol:len() == 3 then
        vol = ((tonumber(vol))/10)
    end
    if ((tonumber(vol)) >= 99) then
        vol = 0
    end
    luup.variable_set(VOS_VID, "Volume", vol, dev)
end

--------------------------------------------------------------------------------
local function set_mute(dev, val)
    local mute_map = { OFF = '0', ON = '1' }
    luup.variable_set(VOS_VID, "Mute", mute_map[val], dev)
end

--------------------------------------------------------------------------------
local function set_power(dev, val)
    local pw_map = { OFF = '0', ON = '1' }
    luup.variable_set(SWP_SID, SWP_STATUS, pw_map[val], dev)
    log("Power" .. val .. tostring(dev))
end

--------------------------------------------------------------------------------
local function set_source(dev, source)
    luup.variable_set(IPS_IID, "Input", source, dev)
end

--------------------------------------------------------------------------------
local function set_source_name(source, name)
    log("(handlerFunction) data received SS source:" .. source .. ":source name:" .. name ..":")
    sourceName[source] = name
end

--------------------------------------------------------------------------------
local function set_surround_mode(dev, mode)
    luup.variable_set(IPS_IID, "Surround", mode, dev)
end

--------------------------------------------------------------------------------
local function set_model(modelName)
    local zone1 = zoneNameTable.R1 or ''
    luup.attr_set("name", modelName, denonDevice)
    log('Setting Model To' .. modelName);
end

--------------------------------------------------------------------------------
local function set_zone_name(zone, name)
    zoneNameTable[zone] = name
end

--------------------------------------------------------------------------------
local function set_video_source(dev, source)
    luup.variable_set(IPS_IID,"Video", source, dev)
end

--------------------------------------------------------------------------------
local cmds = {

    -- MAIN ZONE:
    {'^MV([%d]+)',        function (...)  set_volume        (findChild('Z1'),  ...) end}, -- MV<VOL>
    {'^MU([%w]+)',        function (...)  set_mute          (findChild('Z1'),  ...) end}, -- MUON / MUOFF
    {'^ZM([%w]+)',        function (...)  set_power         (findChild('Z1'),  ...) end}, -- ZMON / ZMOFF
    {'^SI(.+)',           function (...)  set_source        (findChild('Z1'),  ...) end}, -- SI<source>
    {'^MS(.+)',           function (...)  set_surround_mode (findChild('Z1'), ...)  end}, -- MS<surround_mode>
    {'^SV(.+)',           function (...)  set_video_source  (findChild('Z1'), ...)  end}, -- SV<video_source>

    -- Other Zones:
    {'^(Z[%d])([%d]+)',   function (d, ...) set_volume       (findChild(d), ...) end}, -- Z?<VOL>
    {'^(Z[%d])(ON)',      function (d, ...) set_power        (findChild(d), ...) end}, -- Z?ON
    {'^(Z[%d])(OFF)',     function (d, ...) set_power        (findChild(d), ...) end}, -- Z?OFF
    {'^(Z[%d])MU([%w]+)', function (d, ...) set_mute         (findChild(d), ...) end}, -- Z?MUON / Z?MUOFF
    {'^(Z[%d])SI(.+)',    function (d, ...) set_source       (findChild(d), ...) end}, -- Z?SI<source>
    {'^(Z[%s])MS(.+)',    function (d, ...) set_surround_mode(findChild(d), ...) end}, -- Z?MS<surround_mode>
    {'^(Z[%s])SV(.+)',    function (d, ...) set_video_source (findChild(d), ...) end}, -- Z?SV<video_source>

    -- Generic Receiver:
    {'^SSFUN([%w]+)%s(.+)', set_source_name},  -- note: source names can have spaces!
    {'^SYMO([%S]+)',        set_model},     -- I'm assuming no spaces, but some special chars
    {'^(R[%d])([%S].+)',    set_zone_name}, -- note: zone names can have spaces!
    {'^PWON',               function(...) set_power(denonDevice, 'ON')  end},
    {'^PWSTANDBY',          function(...) set_power(denonDevice, 'OFF') end},

}

--------------------------------------------------------------------------------
local function try (func, m, ...)
    if nil == m then return false end
    -- luup.log ('match: '..table.concat ({m,...}, ' '))
    func (m, ...)
    return true
end

--------------------------------------------------------------------------------
function incoming_data(data)
    -- log("(incoming_data) received " .. data)

    for _,cmd in ipairs (cmds) do
        if try (cmd[2], data:match(cmd[1])) then
            return
        end
    end

    log("UNHANDLED RETURN: " .. data)
end
