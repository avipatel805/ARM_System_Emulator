/**************************************************************************
 * C S 429 system emulator
 * 
 * instr_Execute.c - Execute stage of instruction processing pipeline.
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

extern bool X_condval;

extern comb_logic_t copy_m_ctl_sigs(m_ctl_sigs_t *, m_ctl_sigs_t *);
extern comb_logic_t copy_w_ctl_sigs(w_ctl_sigs_t *, w_ctl_sigs_t *);

/*
 * Execute stage logic.
 * STUDENT TO-DO:
 * Implement the execute stage.
 * 
 * Use in as the input pipeline register,
 * and update the out pipeline register as output.
 * 
 * You will need the following helper functions:
 * copy_m_ctl_signals, copy_w_ctl_signals, and alu.
 */

comb_logic_t execute_instr(x_instr_impl_t *in, m_instr_impl_t *out) {
    if ((in->status == STAT_AOK || in->status == STAT_BUB)&& M_out->status != STAT_HLT && W_out->status != STAT_HLT) {
        uint64_t alu_valb;
        if(in->X_sigs.valb_sel){
            alu_valb = in->val_b;
        }
        else{
            alu_valb = in->val_imm;
        }
        uint64_t res;
        bool cond_val;
        alu(in->val_a, alu_valb, in->val_hw, in->ALU_op, in->X_sigs.set_CC, in->cond, &res, &cond_val, &(guest.proc->NZCV));

        out->op = in->op;
        out->print_op = in->print_op;
        out->seq_succ_PC = in->seq_succ_PC;
        out->cond_holds = cond_val;
        copy_m_ctl_sigs(&(out->M_sigs), &(in->M_sigs));
        copy_w_ctl_sigs(&(out->W_sigs), &(in->W_sigs));
        out->val_b = in->val_b;
        out->dst = in->dst;
        out->val_ex = res;
        
        if(in->op == OP_BL || in->op == OP_BLR){
            out->val_ex = out->seq_succ_PC;
        } else if (in->op == OP_CBZ) {
            out->cond_holds = out->val_ex == 0;
        } else if (in->op == OP_CBNZ) {
            out->cond_holds = out->val_ex != 0;
        }
        out->status = in->status;
    } else {
        out->op = in->op;
        out->status = in->status;
        out->print_op = in->print_op;
    }
    return;
}
