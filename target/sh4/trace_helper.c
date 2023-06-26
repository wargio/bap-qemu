#include "tracewrap.h"
#include "cpu.h"
#include "qemu/log.h"
#include "exec/helper-proto.h"
#include "exec/memop.h"

const char *regs[] = {
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
    "r0b", "r1b", "r2b", "r3b", "r4b", "r5b", "r6b", "r7b",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "sr",
    "gbr", "ssr", "spc", "sgr", "dbr", "vbr", "mach", "macl", "pr"
};
static const int reg_max = sizeof(regs) / sizeof(regs[0]);

void HELPER(trace_newframe)(target_ulong pc)
{
	qemu_trace_newframe(pc, 0);
}

void HELPER(trace_endframe)(CPUSH4State *env, target_ulong old_pc)
{
	qemu_trace_endframe(env, old_pc, 32);
}

OperandInfo * load_store_reg(uint32_t reg, uint32_t val, int ls)
{
    RegOperand * ro = g_new(RegOperand,1);
    reg_operand__init(ro);
    ro->name = g_strdup(reg < reg_max ? regs[reg] : "UNKNOWN");

    OperandInfoSpecific *ois = g_new(OperandInfoSpecific,1);
    operand_info_specific__init(ois);
    ois->reg_operand = ro;
    OperandUsage *ou = g_new(OperandUsage,1);
    operand_usage__init(ou);
    if (ls == 0)
    {
        ou->read = 1;
    } else {
        ou->written = 1;
    }
    OperandInfo *oi = g_new(OperandInfo,1);
    operand_info__init(oi);
    oi->bit_length = 0;
    oi->operand_info_specific = ois;
    oi->operand_usage = ou;
    oi->value.len = 4;
    oi->value.data = g_malloc(oi->value.len);
    memcpy(oi->value.data, &val, 4);
    return oi;
}

void HELPER(trace_load_reg)(uint32_t reg, uint32_t val)
{
    // qemu_log("This register (r%d) was read. Value 0x%x\n", reg, val);

    //r0 always reads 0
    OperandInfo *oi = load_store_reg(reg, (reg != 0) ? val : 0, 0);

    qemu_trace_add_operand(oi, 0x1);
}

void HELPER(trace_store_reg)(uint32_t reg, uint32_t val)
{
    // qemu_log("This register (r%d) was written. Value: 0x%x\n", reg, val);

    OperandInfo *oi = load_store_reg(reg, val, 1);

    qemu_trace_add_operand(oi, 0x2);
}

OperandInfo *load_store_mem(uint64_t addr, int ls, const void *data, size_t data_size) {
    MemOperand * mo = g_new(MemOperand, 1);
    mem_operand__init(mo);

    mo->address = addr;

    OperandInfoSpecific *ois = g_new(OperandInfoSpecific, 1);
    operand_info_specific__init(ois);
    ois->mem_operand = mo;

    OperandUsage *ou = g_new(OperandUsage, 1);
    operand_usage__init(ou);
    if (ls == 0) {
        ou->read = 1;
    } else {
        ou->written = 1;
    }
    OperandInfo *oi = g_new(OperandInfo, 1);
    operand_info__init(oi);
    oi->bit_length = data_size * 8;
    oi->operand_info_specific = ois;
    oi->operand_usage = ou;
    oi->value.len = data_size;
    oi->value.data = g_malloc(oi->value.len);
    #ifdef BSWAP_NEEDED
    memcpy_rev(oi->value.data, data, data_size);
    #else
    memcpy(oi->value.data, data, data_size);
    #endif
    return oi;
}

void HELPER(trace_ld)(CPUSH4State *env, uint32_t val, uint32_t addr)
{
    // qemu_log("This was a read 0x%x addr:0x%x value:0x%x\n", env->active_tc.PC, addr, val);

    OperandInfo *oi = load_store_mem(addr, 0, val, 4);

    qemu_trace_add_operand(oi, 0x1);
}

void HELPER(trace_st)(CPUSH4State *env, uint32_t val, uint32_t addr)
{
    // qemu_log("This was a store 0x%x addr:0x%x value:0x%x\n", env->active_tc.PC, addr, val);

    OperandInfo *oi = load_store_mem(addr, 1, val, 4);

    qemu_trace_add_operand(oi, 0x2);
}
