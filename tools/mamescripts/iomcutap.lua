-- Logs MIPS commands to IOMCU and what the MCU answers
--
--   HNG64_TAP_LOG=iomcu.log ./path/to/mame/hng64 roadedge -rompath games \
--     -autoboot_script tools/mamescripts/iomcutap.lua
--
-- Used this for RE and behavior analysis
local DP = 0x1f808000
local KICK = 0x1f7021c4
local DP_STATUS = 0x002
local DP_DETAIL = 0x003
local DP_HEAD = 0x004
local DP_BLOCKS = 0x010
local DP_DATA = 0x200
local DP_EVENT = 0x402
local DP_EEPROM = 0x600
local STATUS_BUSY = 0x02
local CMD_SIZES = 0x06
local CMD_CONFIG = 0x0a
local CMD_EEPROM = 0x0b
local CONFIG_SIZES = 0x04
local EEPROM_LEN = 0x100

-- bytes each command reads from DP 0x200
local DATA_LEN = {
    [0x05] = 24,
    [0x06] = 4,
    [0x09] = 4,
    [0x0a] = 64,
    [0x0d] = 24
}

-- MCU port 7 bits from hng64.cpp ioport7_w
local P7_RESET = 0x01
local P7_CLOCK = 0x02
local P7_PAGE = 0x0c
local P7_IRQ = 0x80
local MCU_PORT0 = 0x0000
local MCU_PORT7 = 0x0007

local machine = manager.machine
local main = machine.devices[":maincpu"]
local main_mem = main.spaces["program"]
local mcu_mem = machine.devices[":iomcu"].spaces["program"]
local screen = machine.screens[":screen"]
local log = assert(io.open(os.getenv("HNG64_TAP_LOG") or "iomcutap.log", "w"))

-- digital, analog, output 1, output 2
local sizes = {0, 0, 0, 0}

local port7 = 0
local counter = 0
local last_status = 0

log:setvbuf("line")

local function emit(fmt, ...)
    local now = machine.time:as_double()
    local frame = screen:frame_number()

    log:write(string.format("%.6f %d " .. fmt .. "\n", now, frame, ...))
end

local function dp_read(addr)
    return main_mem:read_u8(DP + addr)
end

local function dp_hex(addr, len)
    local out = {}

    for i = 0, len - 1 do
        out[#out + 1] = string.format("%02X", dp_read(addr + i))
    end
    return table.concat(out)
end

local function block_field(name, addr, len)
    if len == 0 then
        return ""
    end

    return string.format(" %s=%s", name, dp_hex(addr, len))
end

local function read_sizes(addr)
    for i = 1, 4 do
        sizes[i] = dp_read(addr + i - 1)
    end
end

local function on_kick(offset, value, mask)
    -- See mips_to_iomcu_irq_w
    if (mask & 0xffff0000) == 0 then
        return
    end

    local cmd = dp_read(0)
    local par = dp_read(1)
    local ev = dp_read(DP_EVENT)
    local pc = main.state["PC"].value & 0xffffffff
    local data_len = DATA_LEN[cmd] or 0
    local eeprom_len = 0

    if cmd == CMD_SIZES then
        read_sizes(DP_DATA)
    end

    if cmd == CMD_CONFIG then
        read_sizes(DP_DATA + CONFIG_SIZES)
    end

    if cmd == CMD_EEPROM then
        eeprom_len = EEPROM_LEN
    end

    local out1 = DP_BLOCKS + sizes[1] + sizes[2]
    local out2 = out1 + sizes[3]

    emit("kick cmd=%02X par=%02X ev=%02X pc=%08X%s%s%s%s", cmd, par, ev, pc, block_field("data", DP_DATA, data_len),
        block_field("eeprom", DP_EEPROM, eeprom_len), block_field("out1", out1, sizes[3]),
        block_field("out2", out2, sizes[4]))
end

local function on_dp_write(addr, value)
    if addr == DP_STATUS then
        last_status = value
        return
    end

    if addr ~= DP_DETAIL or last_status == STATUS_BUSY then
        return
    end

    local ana = DP_BLOCKS + sizes[1]

    emit("answer status=%02X detail=%02X head=%s%s%s", last_status, value, dp_hex(DP_HEAD, DP_BLOCKS - DP_HEAD),
        block_field("dig", DP_BLOCKS, sizes[1]), block_field("ana", ana, sizes[2]))
end

local function on_port7(value)
    local falling = port7 & ~value

    if (falling & P7_RESET) ~= 0 then
        counter = 0
    end

    if (falling & P7_CLOCK) ~= 0 then
        counter = (counter + 1) & 0x1ff
    end

    port7 = value

    -- mailbox written before the pulse
    if (falling & P7_IRQ) ~= 0 then
        emit("event mbox=%s", dp_hex(DP_EVENT, 4))
    end
end

local function on_mcu_port(offset, value, mask)
    if offset == MCU_PORT7 then
        on_port7(value & 0xff)
        return
    end

    if offset == MCU_PORT0 then
        local addr = counter | ((port7 & P7_PAGE) << 7)
        on_dp_write(addr, value & 0xff)
    end
end

iomcutap_handles = {main_mem:install_write_tap(KICK, KICK + 3, "iomcutap_kick", on_kick),
                    mcu_mem:install_write_tap(MCU_PORT0, MCU_PORT7, "iomcutap_port", on_mcu_port)}

emu.add_machine_stop_notifier(function()
    log:close()
end)
