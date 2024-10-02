/**************************************************************************
 * C S 429 system emulator
 * 
 * Student TODO: AE
 * 
 * hw_elts.c - Module for emulating hardware elements.
 * 
 * Copyright (c) 2022, 2023. 
 * Authors: S. Chatterjee, Z. Leeper. 
 * All rights reserved.
 * May not be used, modified, or copied without permission.
 **************************************************************************/ 

#include <assert.h>
#include "hw_elts.h"
#include "mem.h"
#include "machine.h"
#include "forward.h"
#include "err_handler.h"

extern machine_t guest;

comb_logic_t 
imem(uint64_t imem_addr,
     uint32_t *imem_rval, bool *imem_err) {
    // imem_addr must be in "instruction memory" and a multiple of 4
    *imem_err = (!addr_in_imem(imem_addr) || (imem_addr & 0x3U));
    *imem_rval = (uint32_t) mem_read_I(imem_addr);
}

/*
 * Regfile logic.
 * STUDENT TO-DO:
 * Read from source registers and write to destination registers when enabled.
 */
comb_logic_t
regfile(uint8_t src1, uint8_t src2, uint8_t dst, uint64_t val_w,
        // bool src1_31isSP, bool src2_31isSP, bool dst_31isSP, 
        bool w_enable,
        uint64_t *val_a, uint64_t *val_b) {
    
    //We are reading the values inside of register src1 and src2
    *val_a = guest.proc->GPR[src1];
    *val_b = guest.proc->GPR[src2];
    if (src1 == 31) *val_a = guest.proc->SP;
    if (src2 == 31) *val_b = guest.proc->SP;
    if (src1 > 31) *val_a = 0;
    if (src2 > 31) *val_b = 0;

    if(w_enable){
        guest.proc->GPR[dst] = val_w;
    }
    /*
    int32_t mem_rv;
    bool error;
    imem(guest.mem, mem_rv, error);
    if(bitfield_u32(mem_rv, 26, 6) == 5 || bitfield_u32(mem_rv, 26, 6) == 37){
        //b1
        int32_t imm26 = bitfield_u32(mem_rv, 0, 26);
        guest.proc->GPR[30] = imm26;
        guest.proc->PC = imm26;
    }
    else if(bitfield_u32(mem_rv, 24, 8) == 84){
        //b2
        int32_t imm19 = bitfield_u32(mem_rv, 5, 19);
        guest.proc->GPR[30] = imm19;
        guest.proc->PC = imm19;
    }
    else if(bitfield_u32(mem_rv, 21, 11) == 1714){
        //b3
        int32_t Rn = bitfield_u32(mem_rv, 5, 5);
        guest.proc->GPR[30] = Rn;
        guest.proc->PC = Rn;

    }
    else if(bitfield_u32(mem_rv, 21, 11) == 1986 || bitfield_u32(mem_rv, 21, 11) == 1984){
        //M
        int32_t Rm = bitfield_u32(mem_rv, 16, 5);
        int32_t Rn = bitfield_u32(mem_rv, 5, 5);
        int32_t Rd = bitfield_u32(mem_rv, 0, 5);
        guest.proc->GPR[src1] = Rm;
        guest.proc->GPR[src2] = Rn;
        guest.proc->PC += 4;
    }
    else if(bitfield_u32(mem_rv, 23, 9) == 485 || bitfield_u32(mem_rv, 23, 9) == 421){
        //I
        guest.proc->PC += 4;
    }
    else if(bitfield_u32(mem_rv, 21, 11) == 1704 || bitfield_u32(mem_rv, 21, 11) == 1698){
        //S
        guest.proc->PC += 4;
    }
    else if(bitfield_u32(mem_rv, 22, 1) == 1){
        //RI
        guest.proc->GPR[src1] = bitfield_u32(mem_rv, 5, 5);

    }
    else{
        //RR
    }
    */
}

/*
 * cond_holds logic.
 * STUDENT TO-DO:
 * Determine if the condition is true or false based on the condition code values
 */
static bool cond_holds(cond_t cond, uint8_t ccval) {
    if (cond == C_EQ && GET_ZF(ccval)) return true;
    if (cond == C_NE && !GET_ZF(ccval)) return true;
    if (cond == C_CS && GET_CF(ccval)) return true;
    if (cond == C_CC && !GET_CF(ccval)) return true;
    if (cond == C_MI && GET_NF(ccval)) return true;
    if (cond == C_PL && !GET_NF(ccval)) return true;
    if (cond == C_VS && GET_VF(ccval)) return true;
    if (cond == C_VC && !GET_VF(ccval)) return true;

    if (cond == C_HI && !GET_ZF(ccval) && GET_CF(ccval)) return true;
    if (cond == C_LS && !(!GET_ZF(ccval) && GET_CF(ccval))) return true;

    if (cond == C_GE && GET_NF(ccval) == GET_VF(ccval)) return true;
    if (cond == C_LT && !(GET_NF(ccval) == GET_VF(ccval))) return true;
    if (cond == C_GT && !GET_ZF(ccval) && GET_NF(ccval) == GET_VF(ccval)) return true;
    if (cond == C_LE && !(!GET_ZF(ccval) && GET_NF(ccval) == GET_VF(ccval))) return true;

    if (cond == C_AL || cond == C_NV) return true;
    return false;
}

/*
 * alu logic.
 * STUDENT TO-DO:
 * Compute the result of a bitwise or mathematial operation (all operations in alu_op_t).
 * Additionally, apply hw or compute condition code values as necessary.
 * Finally, compute condition values with cond_holds.
 */
comb_logic_t 
alu(uint64_t alu_vala, uint64_t alu_valb, uint8_t alu_valhw, alu_op_t ALUop, bool set_CC, cond_t cond, 
    uint64_t *val_e, bool *cond_val, uint8_t *nzcv) {
    uint64_t res = 0xFEEDFACEDEADBEEF;  // To make it easier to detect errors.
    
    switch (ALUop) {
        case PLUS_OP:
            *val_e = alu_vala + (alu_valb << alu_valhw);
            break;
        case MINUS_OP:
            *val_e = alu_vala - (alu_valb << alu_valhw);
            break;
        case INV_OP:
            *val_e = alu_vala | (~alu_valb << alu_valhw);   
            break;
        case OR_OP:
            *val_e = alu_vala | alu_valb;
            break;
        case EOR_OP:
            *val_e = alu_vala ^ alu_valb;
            break;
        case AND_OP:
            *val_e = alu_vala & alu_valb; 
            break;
        case MOV_OP:
            *val_e = alu_vala | (alu_valb << alu_valhw); 
            break;
        case LSL_OP:
            *val_e = alu_vala << (alu_valb & 0x3FUL); 
            break;
        case LSR_OP:
            *val_e = alu_vala >> (alu_valb & 0x3FUL);
            break;
        case ASR_OP:
            *val_e = ((int64_t) alu_vala) >> (alu_valb & 0x3FUL); 
            break;
        case PASS_A_OP:
            *val_e = alu_vala; 
            break;
        case CBZ_OP:
            *val_e = alu_vala; 
            cond_val = alu_vala == 0;
            break;
        case CBNZ_OP:
            *val_e = alu_vala; 
            cond_val = alu_vala != 0;
            break;
        case CSEL_OP:
            if (cond_holds(cond, *nzcv)) {
                *val_e = alu_vala;
                }
            else {
                *val_e = alu_valb;
                }
            break;
        case CSINV_OP:
            if (cond_holds(cond, *nzcv)) {
                *val_e = alu_vala;
                }
            else {
                *val_e = ~alu_valb;
                }
            break;
        case CSINC_OP:
            if (cond_holds(cond, *nzcv)) {
                *val_e = alu_vala;
                }
            else {
                *val_e = alu_valb + 1;
                }
            break;
        case CSNEG_OP:
            if (cond_holds(cond, *nzcv)) {
                *val_e = alu_vala;
                }
            else {
                *val_e = alu_valb * -1;
                }
            break;
        case ERROR_OP:
            *val_e = res;
            break;
    }
    
    if (set_CC) {
        *nzcv = 0;
        if ((*val_e >> 63) & 1)  *nzcv += 8;
        if (*val_e == 0)  *nzcv += 4;
        if (ALUop == PLUS_OP) {
            if (*val_e < alu_vala) *nzcv += 2;

            if(((alu_vala >> 63) == (alu_valb >> 63)) && ((alu_vala >> 63) != (*val_e >> 63))){
                *nzcv += 1;
            }
        } else if (ALUop == MINUS_OP) {
            if (alu_vala >= *val_e) *nzcv += 2;
        }
        
    }
    *cond_val = cond_holds(cond, *nzcv);
}

comb_logic_t 
dmem(uint64_t dmem_addr, uint64_t dmem_wval, bool dmem_read, bool dmem_write, 
     uint64_t *dmem_rval, bool *dmem_err) {
    // dmem_addr must be in "data memory" and a multiple of 8
    *dmem_err = (!addr_in_dmem(dmem_addr) || (dmem_addr & 0x7U));
    if (is_special_addr(dmem_addr)) *dmem_err = false;
    if (dmem_read) *dmem_rval = (uint64_t) mem_read_L(dmem_addr);
    if (dmem_write) mem_write_L(dmem_addr, dmem_wval);
}