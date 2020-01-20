
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
int ENABLE_DEBUG_MESSAGES=1;

//int zero_flag=0;
//int mem_address[1] = 200;
/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);
  cpu->stage[F].halt=0;

  //  TO make all registers valid for initial executions
  for(int i=0;i<16;i++)
  {
    cpu->regs_valid[i] = 1;
  }
  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (!ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "LOAD") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "ADDL") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "SUBL") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "ADD") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2 );
  }


  if (strcmp(stage->opcode, "SUB") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2 );
  }


  if (strcmp(stage->opcode, "MUL") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2 );
  }


  if (strcmp(stage->opcode, "AND") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2 );
  }


  if (strcmp(stage->opcode, "OR") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2 );
  }
  if(strcmp(stage->opcode,"")==0){
    printf("");
  }


  if (strcmp(stage->opcode, "EX-OR") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2 );
  }
  if (strcmp(stage->opcode, "BZ") == 0) {
    printf(
      "%s,#%d ", stage->opcode, stage->imm );
  }
  if (strcmp(stage->opcode, "BNZ") == 0) {
    printf(
      "%s,#%d ", stage->opcode, stage->imm );
  }
  if (strcmp(stage->opcode, "JUMP") == 0) {
    printf(
      "%s,R%d,#%d ", stage->opcode,stage->rs1, stage->imm );
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }
  if (strcmp(stage->opcode, "LDR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "STR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  if (strcmp(stage->opcode, "HALT") == 0) {
    printf("%s", stage->opcode);
  }

}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];

  if (!stage->busy && !stage->stalled && !stage->halt) {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    cpu->stage[DRF] = cpu->stage[F];

  if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
    print_stage_content("Fetch", stage);
}
  }
  else if(stage->stalled){
    stage->pc=cpu->pc; // to store non-updated pc
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;
    if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
      print_stage_content("Fetch", stage);
  }
  }
  else if(stage->busy){
    stage->pc=cpu->pc; // to store non-updated pc
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;
    if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
      print_stage_content("Fetch", stage);
  }
  }

  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];
  if (strcmp(stage->opcode, "HALT") == 0) {
    CPU_Stage* stage1 = &cpu->stage[F];//object creation for[F]
    stage1->halt=1;
    cpu->pc +=0;

}

  if(stage->stalled){
    stage->stalled=0;
    cpu->stage[F].stalled=0;
    //cpu->stage[EX1] = cpu->stage[DRF];

  }
  if(stage->busy){
    stage->busy=0;
    cpu->stage[F].busy=0;
  }

  if (!stage->busy && !stage->stalled) {

    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;

      //stage->rdd = cpu->regs[stage->rd];
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      //cpu->regs_valid[stage->rd]=0;
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }


    if (strcmp(stage->opcode, "STR") == 0) {
      if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;

      stage->rdd = cpu->regs[stage->rd];
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      //cpu->regs_valid[stage->rd]=0;
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
      if(cpu->regs_valid[stage->rs1])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;

      stage->rdd = cpu->regs[stage->rd];
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]=0;
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }

    if (strcmp(stage->opcode, "LDR") == 0) {
      if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;

      stage->rdd = cpu->regs[stage->rd];
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]=0;
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }


    if (strcmp(stage->opcode, "ADD") == 0) {
      if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;

      stage->rdd = cpu->regs[stage->rd];
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]=0;
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
      //cpu->stage[EX1].stalled=1;
    }
    }

    if (strcmp(stage->opcode, "ADDL") == 0) {
      if(cpu->regs_valid[stage->rs1])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;

      stage->rdd = cpu->regs[stage->rd];
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]=0;
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }

    if (strcmp(stage->opcode, "SUB") == 0) {
      if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;

      stage->rdd = cpu->regs[stage->rd];
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]=0;
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }
    if (strcmp(stage->opcode, "SUBL") == 0) {
      if(cpu->regs_valid[stage->rs1])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;

      stage->rdd = cpu->regs[stage->rd];
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]=0;
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }
    if (strcmp(stage->opcode, "MUL") == 0) {
      if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;

      stage->rdd = cpu->regs[stage->rd];
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]=0;
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }
    if (strcmp(stage->opcode, "AND") == 0) {
      if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;

      stage->rdd = cpu->regs[stage->rd];
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]=0;
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }
    if (strcmp(stage->opcode, "OR") == 0) {
      if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;

      stage->rdd = cpu->regs[stage->rd];
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]=0;
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }
    if (strcmp(stage->opcode, "EX-OR") == 0) {
      if(cpu->regs_valid[stage->rs1]&&cpu->regs_valid[stage->rs2])
      {
        cpu->stage[F].stalled=0;
        cpu->stage[DRF].stalled=0;

      stage->rdd = cpu->regs[stage->rd];
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
      cpu->regs_valid[stage->rd]=0;
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }
    if (strcmp(stage->opcode, "BZ") == 0) {
      if(!cpu->zero_flag&&!cpu->bz_flag){
      cpu->stage[F].busy=1;
      cpu->stage[DRF].busy=1;
      cpu->stage[EX1].busy=1;
    //  cpu->bz_flag=1;
    }
      if(!cpu->zero_flag&&cpu->bz_flag){
        cpu->stage[F].busy=0;
        cpu->stage[DRF].busy=0;
        cpu->stage[EX1].busy=0;
        //cpu->bz_flag=1;
      }
      /*if(zero_flag){
        stage->stalled=1;
      }*/
      //cpu->stage[F].stalled=1;
      //cpu->stage[EX1].stalled=1;
    }
    if (strcmp(stage->opcode, "BNZ") == 0) {
      if(!cpu->zero_flag&&!cpu->bz_flag){
      cpu->stage[F].busy=1;
      cpu->stage[DRF].busy=1;
      cpu->stage[EX1].busy=1;
      cpu->bz_flag=1;
    }

    }
    if (strcmp(stage->opcode, "JUMP") == 0) {
      if(cpu->regs_valid[stage->rs1])
      {
      stage->rs1_value = cpu->regs[stage->rs1];
    }
    else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }
    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      if(cpu->regs_valid[stage->rd]){
      stage->rdd = cpu->regs[stage->rd];
      cpu->regs_valid[stage->rd]=0;
    }else{
      cpu->stage[F].stalled=1;
      cpu->stage[DRF].stalled=1;
    }
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[EX1] = cpu->stage[DRF];

  if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
    print_stage_content("Decode/RF", stage);
  }
}
return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
execute1(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX1];

  if(stage->stalled){
    cpu->stage[EX2]=cpu->stage[EX1];
    if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
      printf("Execute1       :     \n");
    }
  }
  if(stage->busy){
    cpu->stage[EX2]=cpu->stage[EX1];
    if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
      printf("Execute1       :    \n");
    }
  }
  if (!stage->busy && !stage->stalled) {
    if (strcmp(stage->opcode, "BZ") == 0) {

       }



     if (strcmp(stage->opcode, "BNZ") == 0) {
       if(!cpu->zero_flag&&!cpu->bz_flag){
        cpu->bz_flag=1;
}
      }
      if (strcmp(stage->opcode, "JUMP") == 0) {
        stage->buffer = stage->rs1_value + stage->imm;
        printf("%d\n",cpu->pc );

        int newpc;
        newpc = stage->buffer + (cpu->pc-8);
        cpu->pc = newpc;
        printf("%d\n",cpu->pc );
      }

    if (strcmp(stage->opcode, "ADD") == 0) {
      stage->rdd = stage->rs1_value + stage->rs2_value;
    }
    if (strcmp(stage->opcode, "AND") == 0) {
      stage->rdd = stage->rs1_value & stage->rs2_value;
    }
    if (strcmp(stage->opcode, "SUB") == 0) {
      stage->rdd = stage->rs1_value - stage->rs2_value;
    }
    if (strcmp(stage->opcode, "MUL") == 0) {
      stage->rdd = stage->rs1_value * stage->rs2_value;
    }
    if (strcmp(stage->opcode, "OR") == 0) {
      stage->rdd = stage->rs1_value | stage->rs2_value;
    }
    if (strcmp(stage->opcode, "EX-OR") == 0) {
      stage->rdd = stage->rs1_value ^ stage->rs2_value;
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      stage->imm=stage->imm+0;

    }

    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[EX2] = cpu->stage[EX1];
  }

 if(ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {

    print_stage_content("Execute1", stage);
  }
  return 0;
}

int
execute2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX2];
//  printf("Zero=%d, BZ=%d\n",cpu->zero_flag,cpu->bz_flag );
  if(stage->stalled){
    cpu->stage[MEM1]=cpu->stage[EX2];
    if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
      printf("Execute2       :    \n ");
  }
}
if(stage->busy){
  cpu->stage[MEM1]=cpu->stage[EX2];
  if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
    printf("Execute2       :    \n ");
}
}
  if (!stage->busy && !stage->stalled) {

    if (strcmp(stage->opcode, "ADDL") == 0) {
      stage->rdd = stage->rs1_value + stage->imm;
    }
    if (strcmp(stage->opcode, "SUBL") == 0) {
      stage->rdd = stage->rs1_value - stage->imm;
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      stage->buffer = stage->imm;
    }
    if (strcmp(stage->opcode, "BZ") == 0) {
        cpu->bz_flag=1;
      if(cpu->zero_flag&&cpu->bz_flag){
        strcpy(cpu->stage[F].opcode,"");
        strcpy(cpu->stage[DRF].opcode,"");
        strcpy(cpu->stage[EX1].opcode,"");
        int newpc;
      //  printf("CPUPC-%d",cpu->pc);
        newpc = stage->imm + (cpu->pc-12);
        cpu->pc = newpc;
        if(stage->imm<0){
          cpu->ins_completed=(cpu->ins_completed+(stage->imm/4))-1;
        }
        else{
          cpu->ins_completed=cpu->ins_completed-(stage->imm/4);
        }
        strcpy(cpu->stage[EX2].opcode,"");
        ////printf("PC%d,%d\n",newpc,stage->imm );
       cpu->bz_flag=0;
       cpu->zero_flag=0;
      }
      else if(cpu->zero_flag==0){
        cpu->stage[F].busy=0;
        cpu->stage[DRF].busy=0;
        cpu->pc += 0;
        //cpu->stage[DRF] = cpu->stage[F];

      }


    }
    if (strcmp(stage->opcode, "BNZ") == 0) {
      //cpu->bz_flag=1;
      if(!cpu->zero_flag&&cpu->bz_flag){
        strcpy(cpu->stage[F].opcode,"");
        strcpy(cpu->stage[DRF].opcode,"");
        strcpy(cpu->stage[EX1].opcode,"");
        int newpc;
      //  printf("CPUPC-%d",cpu->pc);
        newpc = stage->imm + (cpu->pc-12);
        cpu->pc = newpc;
        if(stage->imm<0){
          cpu->ins_completed=(cpu->ins_completed+(stage->imm/4))-1;
        }
        else{
          cpu->ins_completed=cpu->ins_completed-(stage->imm/4);
        }
        strcpy(cpu->stage[EX2].opcode,"");
        ////printf("PC%d,%d\n",newpc,stage->imm );
       cpu->bz_flag=0;
       cpu->zero_flag=0;
      }
      else if(cpu->zero_flag==1){
        cpu->stage[F].busy=0;
        cpu->stage[DRF].busy=0;
        cpu->pc += 0;
        //cpu->stage[DRF] = cpu->stage[F];

      }
  printf("%d\n",cpu->zero_flag );

    }

    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM1] = cpu->stage[EX2];
    if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
      print_stage_content("Execute2", stage);
    }
  }
  return 0;
}
/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
memory1(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM1];
  if(stage->stalled){
    cpu->stage[MEM2]=cpu->stage[MEM1];
    if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
      printf("Memory1        :  \n");
  }
}
if(stage->busy){
  cpu->stage[MEM2]=cpu->stage[MEM1];
  if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
    printf("Memory1        :    \n ");
}
}
  if (!stage->busy && !stage->stalled) {

    /* Store */
    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      stage->buffer = stage->rs2_value + stage->imm;
      //printf("value in ex stage %d\n",stage->buffer );
    }
    if (strcmp(stage->opcode, "STR") == 0) {
      stage->buffer = stage->rs1_value + stage->rs2_value;
      //printf("value in ex stage %d\n",stage->buffer );
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
      stage->rdd = stage->rs1_value + stage->imm;
    }
    if (strcmp(stage->opcode, "LDR") == 0) {
      stage->rdd = stage->rs1_value + stage->rs2_value;
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }
    if (strcmp(stage->opcode, "BZ") == 0) {

    }
    /* Copy data from decode latch to execute latch*/
    cpu->stage[MEM2] = cpu->stage[MEM1];

    if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
      print_stage_content("Memory1", stage);
    }
  }
  return 0;
}

int
memory2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM2];
  if(stage->stalled){
    cpu->stage[WB]=cpu->stage[MEM2];
    if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
      printf("Memory2        :   \n ");
  }
}
if(stage->busy){
  cpu->stage[WB]=cpu->stage[MEM2];
  if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
    printf("Memory2        :   \n ");
}
}
  if (!stage->busy && !stage->stalled) {


    /* Store */
    if (strcmp(stage->opcode, "LOAD") == 0) {
      stage->mem_address = stage->rdd;
    }
    if (strcmp(stage->opcode, "LDR") == 0) {
      stage->mem_address = stage->rdd;
    }
    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      stage->mem_address = stage->buffer;
      //printf("value in ex stage %d\n",stage->buffer );
    }
    if (strcmp(stage->opcode, "STR") == 0) {
      stage->mem_address = stage->buffer;
    }
    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }
    if (strcmp(stage->opcode, "BZ") == 0) {
      //cpu->stage[DRF].stalled=0;

    }
    /* Copy data from decode latch to execute latch*/
   cpu->stage[WB] = cpu->stage[MEM2];

    if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
      print_stage_content("Memory2", stage);
    }
  }
  return 0;
}


/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
writeback(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[WB];


  if (strcmp(stage->opcode, "HALT") == 0) {
    if(ENABLE_DEBUG_MESSAGES||cpu->simulate){
    int i ;
    printf("======================================= REGISTERS ==================================\n");
    for(i=0;i<16;i++)
    {
      printf("|  Register %-6d  |  Value= %-6d  |  Status= %-6d  | \n",i,cpu->regs[i],cpu->regs_valid[i] );
    }
    printf("======================================= DATA MEMORY ==================================\n");
    for(i=0;i<99;i++)
    {
      printf("|  Data Memory %-6d  |  Value= %-6d  | \n",i,cpu->data_memory[i] );
    }

}

      APEX_cpu_stop(cpu);
	printf("(apex) >> Simulation Complete\n ======================================= IGNORE BELOW PRINTED INFORMATION==================================\n");
    }

    if(stage->stalled){
        if(!cpu->simulate){
          printf("Writeback      :   \n");
        }
          cpu->stage[F].stalled=0;
          cpu->stage[DRF].stalled=0;
          cpu->stage[EX1].stalled=0;
      }
      if(stage->busy){
        if(!cpu->simulate){
          printf("Writeback      :   \n");
        }
            cpu->stage[F].busy=0;
            cpu->stage[DRF].busy=0;
            cpu->stage[EX1].busy=0;
            strcpy(cpu->stage[F].opcode,"");
        }

  if (!stage->busy && !stage->stalled&&(strcmp(stage->opcode,"")!=0)) {

    /* Update register file */
   if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd]=1;
    }
    if (strcmp(stage->opcode, "ADD") == 0) {
      cpu->regs[stage->rd] = stage->rdd;
      //cpu->bz_flag=0;
      cpu->regs_valid[stage->rd]=1;
      cpu->stage[F].stalled=0;
      cpu->stage[DRF].stalled=0;
      if(cpu->regs[stage->rd]==0){
        cpu->zero_flag = 1;
        //cpu->bz_flag=1;
      }
      else{
        cpu->zero_flag = 0;
        cpu->bz_flag=0;
      }
    }
    if (strcmp(stage->opcode, "AND") == 0) {
      cpu->regs[stage->rd] = stage->rdd;
      cpu->regs_valid[stage->rd]=1;
      cpu->stage[F].stalled=0;
      cpu->stage[DRF].stalled=0;

    }
    if (strcmp(stage->opcode, "OR") == 0) {
      cpu->regs[stage->rd] = stage->rdd;
      cpu->regs_valid[stage->rd]=1;
      cpu->stage[F].stalled=0;
      cpu->stage[DRF].stalled=0;

    }
    if (strcmp(stage->opcode, "EX-OR") == 0) {
      cpu->regs[stage->rd] = stage->rdd;
      cpu->regs_valid[stage->rd]=1;
      cpu->stage[F].stalled=0;
      cpu->stage[DRF].stalled=0;

    }
    if (strcmp(stage->opcode, "ADDL") == 0) {
      cpu->regs[stage->rd] = stage->rdd;
      cpu->regs_valid[stage->rd]=1;
      cpu->stage[F].stalled=0;
      cpu->stage[DRF].stalled=0;

    }
    if (strcmp(stage->opcode, "SUBL") == 0) {
      cpu->regs[stage->rd] = stage->rdd;
      cpu->regs_valid[stage->rd]=1;
      cpu->stage[F].stalled=0;
      cpu->stage[DRF].stalled=0;

    //  printf("ADD value %d\n",cpu->regs[stage->rd] );
    }
    if (strcmp(stage->opcode, "SUB") == 0) {
      cpu->regs[stage->rd] = stage->rdd;
      cpu->regs_valid[stage->rd]=1;
      cpu->stage[F].stalled=0;
      cpu->stage[DRF].stalled=0;
      if(cpu->regs[stage->rd]==0){
        cpu->zero_flag = 1;
        //cpu->bz_flag=1;
      }
      else{
      cpu->zero_flag = 0;
      cpu->bz_flag=0;
      }
    //  printf("SUB value %d and Zero flag=%d\n",cpu->regs[stage->rd],cpu->zero_flag );
    }
    if (strcmp(stage->opcode, "MUL") == 0) {
      cpu->regs[stage->rd] = stage->rdd;
      cpu->regs_valid[stage->rd]=1;
      cpu->stage[F].stalled=0;
      cpu->stage[DRF].stalled=0;
      if(cpu->regs[stage->rd]==0){
        cpu->zero_flag = 1;
      }
      else{
        cpu->zero_flag = 0;
        cpu->bz_flag=0;
      }
      //printf("MUL value %d\n",cpu->regs[stage->rd] );
    }
    if (strcmp(stage->opcode, "LOAD") == 0) {
      cpu->regs[stage->rd] = cpu->data_memory[stage->mem_address];
      cpu->regs_valid[stage->rd]=1;
      cpu->stage[F].stalled=0;
      cpu->stage[DRF].stalled=0;
      cpu->bz_flag=0;
    }
    if (strcmp(stage->opcode, "LDR") == 0) {
      cpu->regs[stage->rd] = cpu->data_memory[stage->mem_address];
      cpu->regs_valid[stage->rd]=1;
      cpu->stage[F].stalled=0;
      cpu->stage[DRF].stalled=0;
    }
    if (strcmp(stage->opcode, "STORE") == 0) {
      cpu->data_memory[stage->mem_address] =  stage->rs1_value;
    }
    if (strcmp(stage->opcode, "STR") == 0) {
      cpu->data_memory[stage->mem_address] =  stage->rdd;
    }

    cpu->ins_completed++;

    if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
      print_stage_content("Writeback", stage);
    }
  }
  return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu)
{


  while (1) {

    /* All the instructions committed, so exit */
    if (cpu->ins_completed == cpu->code_memory_size) {
      printf("(apex) >> Simulation Complete \n");
    //  printf("%d", cpu->simulate);
    if(cpu->display||cpu->simulate){
      int i;
      printf("======================================= REGISTERS ==================================\n");
  for(i=0;i<16;i++)
  {
    printf("|  Register %-6d  |  Value= %-6d  |  Status= %d \n",i,cpu->regs[i],cpu->regs_valid[i] );
  }
  printf("======================================= DATA MEMORY ==================================\n");
  for(i=0;i<6;i++)
  {
    printf("M%d-%d\n",i,cpu->data_memory[i] );
  }
}
      break;
    }

    if (ENABLE_DEBUG_MESSAGES&&!cpu->simulate) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }

    writeback(cpu);
    memory2(cpu);
    memory1(cpu);
    execute2(cpu);
    execute1(cpu);
    decode(cpu);
    fetch(cpu);

    cpu->clock++;
  }

  return 0;
}
