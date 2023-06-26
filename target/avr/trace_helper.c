#include "tracewrap.h"
#include "cpu.h"
#include "qemu/log.h"
#include "exec/helper-proto.h"
#include "exec/memop.h"

//#include "trace_helper.h"


/*
 * QEMUs helper.
 */

void HELPER(trace_newframe)(uint64_t pc)
{
    qemu_trace_newframe(pc, 0);
}

void HELPER(trace_endframe)(CPUAVRState *state, uint64_t pc, uint32_t size) {
{
    qemu_trace_endframe(state, pc, size);
}

void HELPER(trace_load_mem)(uint32_t addr, uint32_t val, MemOp op)
{
    qemu_log("LOAD at 0x%lx size: %d data: 0x%lx\n", (unsigned long) addr, memop_size(op), (unsigned long) val);
    OperandInfo *oi = load_store_mem(addr, 0, &val, memop_size(op));
    qemu_trace_add_operand(oi, 0x1);
}

void HELPER(trace_store_mem)(uint32_t addr, uint32_t val, MemOp op)
{
    qemu_log("STORE at 0x%lx size: %d data: 0x%lx\n", (unsigned long) addr, memop_size(op), (unsigned long) val);
    OperandInfo *oi = load_store_mem(addr, 1, &val, memop_size(op));
    qemu_trace_add_operand(oi, 0x2);
}

void HELPER(trace_load_reg)(uint32_t reg, uint32_t val)
{
    OperandInfo *oi = load_store_reg(reg, val, 0);
    qemu_trace_add_operand(oi, 0x1);
}

void HELPER(trace_store_reg)(uint32_t reg, uint32_t val)
{
    OperandInfo *oi = load_store_reg(reg, val, 1);
    qemu_trace_add_operand(oi, 0x2);
}

/*
 * Build frames
 *
 * Functions to fill the actual frame data.
 */

/**
 * \brief Builds a new register load/store operand and returns it.
 *
 * \param name The register name.
 * \param ls If set to 0 the usage flag is set to "read". Otherwise the usage flag is set to "written".
 * \param data Data written to the register.
 * \param data_size Size of the data in bytes.
 * \return OperandInfo* Pointer to the operand for a BAP frame.
 */
static OperandInfo *build_load_store_reg_op(const char *name, int ls, const void *data, size_t data_size) {
    RegOperand * ro = g_new(RegOperand, 1);
    reg_operand__init(ro);
    ro->name = strdup(name);

    OperandInfoSpecific *ois = g_new(OperandInfoSpecific, 1);
    operand_info_specific__init(ois);
    ois->reg_operand = ro;

    OperandUsage *ou = g_new(OperandUsage, 1);
    operand_usage__init(ou);
    if (ls == 0) {
        ou->read = 1;
    } else {
        ou->written = 1;
    }
    OperandInfo *oi = g_new(OperandInfo, 1);
    operand_info__init(oi);
    oi->bit_length = 0;
    oi->operand_info_specific = ois;
    oi->operand_usage = ou;
    oi->value.len = data_size;
    oi->value.data = g_malloc(oi->value.len);
    memcpy(oi->value.data, data, data_size);

    return oi;
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

static const char reg_names[NUMBER_OF_CPU_REGISTERS][8] = {
    "r0",  "r1",  "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
    "r8",  "r9",  "r10", "r11", "r12", "r13", "r14", "r15",
    "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
    "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
};

OperandInfo *load_store_reg(uint32_t reg, uint32_t val, int ls) {
    const char *name = reg_names[reg];
    return build_load_store_reg_op(name, ls, &val, sizeof(val));
}
