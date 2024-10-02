/**************************************************************************
 * C S 429 system emulator
 * 
 * Bubble and stall checking logic.
 * STUDENT TO-DO:
 * Implement logic for hazard handling.
 * 
 * handle_hazards is called from proc.c with the appropriate
 * parameters already set, you must implement the logic for it.
 * 
 * You may optionally use the other three helper functions to 
 * make it easier to follow your logic.
 **************************************************************************/ 

#include "machine.h"
#include "hazard_control.h"

extern machine_t guest;
extern mem_status_t dmem_status;

/* Use this method to actually bubble/stall a pipeline stage.
 * Call it in handle_hazards(). Do not modify this code. */
void pipe_control_stage(proc_stage_t stage, bool bubble, bool stall) {
    pipe_reg_t *pipe;
    switch(stage) {
        case S_FETCH: pipe = F_instr; break;
        case S_DECODE: pipe = D_instr; break;
        case S_EXECUTE: pipe = X_instr; break;
        case S_MEMORY: pipe = M_instr; break;
        case S_WBACK: pipe = W_instr; break;
        default: printf("Error: incorrect stage provided to pipe control.\n"); return;
    }
    if (bubble && stall) {
        printf("Error: cannot bubble and stall at the same time.\n");
        pipe->ctl = P_ERROR;
    }
    // If we were previously in an error state, stay there.
    if (pipe->ctl == P_ERROR) return;

    if (bubble) {
        pipe->ctl = P_BUBBLE;
    }
    else if (stall) {
        pipe->ctl = P_STALL;
    }
    else { 
        pipe->ctl = P_LOAD;
    }
}

bool check_ret_hazard(opcode_t D_opcode) {
    if(D_opcode == OP_RET || D_opcode == OP_BR || D_opcode == OP_BLR) return true;
    return false;
}

bool check_mispred_branch_hazard(opcode_t X_opcode, bool X_condval) {
    /*if((X_opcode == OP_B_COND && !(M_in->cond_holds)) 
    || (X_opcode == OP_CBZ && !(M_in->cond_holds))
     || (X_opcode == OP_CBNZ && !(M_in->cond_holds))) return true;
    return false;*/
    if((X_opcode == OP_B_COND && !X_condval) 
    || (X_opcode == OP_CBZ && !X_condval)
     || (X_opcode == OP_CBNZ && !X_condval)) return true;
    return false;
}

bool check_load_use_hazard(opcode_t D_opcode, uint8_t D_src1, uint8_t D_src2,
                           opcode_t X_opcode, uint8_t X_dst) {
    if(X_opcode == OP_LDUR && (X_dst == D_src2 || X_dst == D_src1)){
        return true;
    }
    return false;
}

comb_logic_t handle_hazards(opcode_t D_opcode, uint8_t D_src1, uint8_t D_src2, uint64_t D_val_a, 
                            opcode_t X_opcode, uint8_t X_dst, bool X_condval) {
    /* Students: Change this code */
    // This will need to be updated in week 2, good enough for week 1
    //bool f_stall = F_out->status == STAT_HLT || F_out->status == STAT_INS;
    //pipe_control_stage(S_FETCH, false, f_stall);
    /*
    if (dmem_status == IN_FLIGHT) {
        pipe_control_stage(S_FETCH, false, true);
        pipe_control_stage(S_DECODE, false, true);
        pipe_control_stage(S_EXECUTE, false, true);
        pipe_control_stage(S_MEMORY, false, true);
        pipe_control_stage(S_WBACK, false, true);
        return;
    }
    else if(check_load_use_hazard(D_opcode, D_src1, D_src2, X_opcode, X_dst)){
        pipe_control_stage(S_FETCH, false, true);
        pipe_control_stage(S_DECODE, false, true);
        pipe_control_stage(S_EXECUTE, true, false);
        pipe_control_stage(S_MEMORY, false, false);
        pipe_control_stage(S_WBACK, false, false);
        return;
    }
    
    else if(check_ret_hazard(D_opcode)){
        pipe_control_stage(S_FETCH, false, true);
        pipe_control_stage(S_DECODE, true, false);
        pipe_control_stage(S_EXECUTE, false, false);
        pipe_control_stage(S_MEMORY, false, false);
        pipe_control_stage(S_WBACK, false, false);
        return;
    }
    else if(check_mispred_branch_hazard(X_opcode, X_condval)){
        pipe_control_stage(S_FETCH, false, false);
        pipe_control_stage(S_DECODE, true, false);
        pipe_control_stage(S_EXECUTE, true, false);
        pipe_control_stage(S_MEMORY, false, false);
        pipe_control_stage(S_WBACK, false, false);
        return;
    }

    pipe_control_stage(S_FETCH, false, false);
    pipe_control_stage(S_DECODE, false, false);
    pipe_control_stage(S_EXECUTE, false, false);
    pipe_control_stage(S_MEMORY, false, false);
    pipe_control_stage(S_WBACK, false, false);
    */
    pipe_control_stage(S_FETCH, false, dmem_status == IN_FLIGHT || check_load_use_hazard(D_opcode, D_src1, D_src2, X_opcode, X_dst) || check_ret_hazard(D_opcode));
    pipe_control_stage(S_DECODE, (check_ret_hazard(D_opcode) || check_mispred_branch_hazard(X_opcode, X_condval)) && !(dmem_status == IN_FLIGHT || check_load_use_hazard(D_opcode, D_src1, D_src2, X_opcode, X_dst)), dmem_status == IN_FLIGHT || check_load_use_hazard(D_opcode, D_src1, D_src2, X_opcode, X_dst));
    pipe_control_stage(S_EXECUTE, (check_load_use_hazard(D_opcode, D_src1, D_src2, X_opcode, X_dst) || check_mispred_branch_hazard(X_opcode, X_condval)) && !(dmem_status == IN_FLIGHT), dmem_status == IN_FLIGHT);
    pipe_control_stage(S_MEMORY, false, dmem_status == IN_FLIGHT);
    pipe_control_stage(S_WBACK, false, dmem_status == IN_FLIGHT);
}