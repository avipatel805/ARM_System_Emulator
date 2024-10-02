/**************************************************************************
 * C S 429 system emulator
 * 
 * instr_Fetch.c - Fetch stage of instruction processing pipeline.
 **************************************************************************/ 

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "err_handler.h"
#include "instr.h"
#include "instr_pipeline.h"
#include "machine.h"
#include "hw_elts.h"

extern machine_t guest;
extern mem_status_t dmem_status;

extern uint64_t F_PC;

/*
 * Select PC logic.
 * STUDENT TO-DO:
 * Write the next PC to *current_PC.
 */

static comb_logic_t 
select_PC(uint64_t pred_PC,                                       // The predicted PC
          opcode_t D_opcode, uint64_t val_a, uint64_t D_seq_succ, // Possible correction from RET
          opcode_t M_opcode, bool M_cond_val, uint64_t seq_succ,  // Possible correction from B.cond
          uint64_t *current_PC) {
    /* 
     * Students: Please leave this code
     * at the top of this function. 
     * You may modify below it. 
     */
    if (D_opcode == OP_RET && val_a == RET_FROM_MAIN_ADDR) {
        *current_PC = 0; // PC can't be 0 normally.
    }
    // Modify starting here.
    else if(D_opcode == OP_RET || D_opcode == OP_BR || D_opcode == OP_BLR){
        *current_PC = val_a;
    }
    else if(!M_cond_val && (M_opcode == OP_B_COND || M_opcode == OP_CBZ || M_opcode == OP_CBNZ)) {
        *current_PC = seq_succ;
    }
    else{
        *current_PC = pred_PC;
    }
    return;
}

/*
 * Predict PC logic. Conditional branches are predicted taken.
 * STUDENT TO-DO:
 * Write the predicted next PC to *predicted_PC
 * and the next sequential pc to *seq_succ.
 */

static comb_logic_t 
predict_PC(uint64_t current_PC, uint32_t insnbits, opcode_t op, 
           uint64_t *predicted_PC, uint64_t *seq_succ) {
    /* 
     * Students: Please leave this code
     * at the top of this function. 
     * You may modify below it. 
     */
    if (!current_PC) {
        return; // We use this to generate a halt instruction.
    }
    // Modify starting here.
    else if (op == OP_B || op == OP_BL) {
        int new_int = (insnbits & 0x03FFFFFF) << 2;
        if ((new_int >> 27) != 0) {
            new_int = new_int | 0xF8000000;
        }
        *predicted_PC = current_PC + new_int;
        *seq_succ = current_PC + 4;
    } else if (op == OP_B_COND) {
        int new_int = (insnbits & 0xFFFFE0) >> 3;
        if ((new_int & 0x100000) != 0) {
            new_int = new_int | 0xFFE00000; 
        } 
        *predicted_PC = current_PC + new_int;
        //*predicted_PC = current_PC + bitfield_u32(insnbits, 5, 19);
    } else if (op == OP_CBZ || op == OP_CBNZ){
        int new_int = (bitfield_s64(insnbits, 5, 19)) << 2;

        *predicted_PC = current_PC + new_int;
    }
    /*
    else if(op == OP_BLR){
        int br = bitfield_u32(insnbits, 5, 5);
        *predicted_PC = guest.proc->GPR[br];
    }
    */
    else {
        *predicted_PC = current_PC + 4;
    }
    *seq_succ = current_PC + 4;
    return;
}

/*
 * Helper function to recognize the aliased instructions:
 * LSL, LSR, CMP, and TST. We do this only to simplify the 
 * implementations of the shift operations (rather than having
 * to implement UBFM in full).
 * STUDENT TO-DO
 */

static
void fix_instr_aliases(uint32_t insnbits, opcode_t *op) {
    if (*op == OP_UBFM) {
        if (bitfield_u32(insnbits, 10, 6) == 63) {
            *op = 17; //LSR
        } else {
            *op = 16; //LSL
        }
    } else if ((*op == OP_SUBS_RR || *op == OP_ANDS_RR) && bitfield_u32(insnbits, 0, 5) == 31) {
        *op += 1;
    } else if (*op == OP_CSEL && bitfield_u32(insnbits, 10, 2)) {
        *op += 2;
    } else if (*op == OP_CSNEG && !bitfield_u32(insnbits, 10, 2)) {
        *op -= 2;
    }
    return;
}

/*
 * Fetch stage logic.
 * STUDENT TO-DO:
 * Implement the fetch stage.
 * 
 * Use in as the input pipeline register,
 * and update the out pipeline register as output.
 * Additionally, update F_PC for the next
 * cycle's predicted PC.
 * 
 * You will also need the following helper functions:
 * select_pc, predict_pc, and imem.
 */

comb_logic_t fetch_instr(f_instr_impl_t *in, d_instr_impl_t *out) {
    bool imem_err = 0;
    uint64_t current_PC;
    select_PC(in->pred_PC, X_out->op, X_out->val_a, X_out->seq_succ_PC,
              M_out->op, M_out->cond_holds, M_out->seq_succ_PC, 
              &current_PC);
    /* 
     * Students: This case is for generating HLT instructions
     * to stop the pipeline. Only write your code in the **else** case. 
     */
    if (!current_PC || F_in->status == STAT_HLT) {
        out->insnbits = 0xD4400000U;
        out->op = OP_HLT;
        out->print_op = OP_HLT;
        imem_err = false;
    }
    else {
        uint32_t insnbits = mem_read_I(current_PC);
        opcode_t op = itable[bitfield_u32(insnbits, 21, 11)];
        fix_instr_aliases(insnbits, &op);
        imem(current_PC, &insnbits, &imem_err);

        if (imem_err || op == OP_ERROR) {
            in->status = STAT_INS;
            out->status = STAT_INS;
            out->insnbits = insnbits;
            out->op = op;
            out->print_op = op;
            out->seq_succ_PC = current_PC + 4;
            F_PC = out->seq_succ_PC;
        } else {
            out->status = STAT_AOK;
            out->op = op;
            out->print_op = op;
            out->insnbits = insnbits;
            out->adrp_val = op == OP_ADRP ? (current_PC >> 12) << 12 : 0;
            if(M_in->status == STAT_AOK || M_in->status == STAT_BUB){
                predict_PC(current_PC, insnbits, op, &F_PC, &(out->seq_succ_PC));
            }
            //if (M_in->status == STAT_INS) out->seq_succ_PC -= 4;
        }
    }
    
    if(M_in->status != STAT_AOK && M_in->status != STAT_BUB){
        out->seq_succ_PC = current_PC;
        F_PC = current_PC;
    }
    
    if (imem_err || out->op == OP_ERROR) {
        in->status = STAT_INS;
        F_in->status = in->status;
    }
    else if (out->op == OP_HLT) {
        in->status = STAT_HLT;
        F_in->status = in->status;
    }
    else {
        in->status = STAT_AOK;
    }
    out->status = in->status;

    return;
}
