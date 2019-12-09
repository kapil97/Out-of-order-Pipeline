/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU *APEX_cpu_init(const char *filename)
{
  if (!filename)
  {
    return NULL;
  }

  APEX_CPU *cpu = malloc(sizeof(*cpu));
  if (!cpu)
  {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->freeRegisterFlag, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory)
  {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES)
  {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i)
    {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i)
  {
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
void APEX_cpu_stop(APEX_CPU *cpu)
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
int get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage *stage)
{

  if (
      strcmp(stage->opcode, "ADD") == 0 ||
      strcmp(stage->opcode, "SUB") == 0 ||
      strcmp(stage->opcode, "MUL") == 0 ||
      strcmp(stage->opcode, "AND") == 0 ||
      strcmp(stage->opcode, "OR") == 0 ||
      strcmp(stage->opcode, "XOR") == 0)
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "LOAD") == 0)
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "STORE") == 0)
  {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0)
  {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }

  if (strcmp(stage->opcode, "HALT") == 0)
  {
    printf("%s  ", stage->opcode);
  }

  if (strcmp(stage->opcode, "BZ") == 0)
  {
    printf("%s,#%d ", stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "LDR") == 0)
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "STR") == 0)
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "ADDL") == 0)
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "SUBL") == 0)
  {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "JUMP") == 0)
  {
    printf("%s,R%d,#%d ", stage->opcode, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "BNZ") == 0)
  {
    printf("%s,#%d ", stage->opcode, stage->imm);
  }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char *name, CPU_Stage *stage)
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
int fetch(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[F];
  if (stage->flush == 1)
  {
    strcpy(stage->opcode, " ");
  }
  if (!stage->busy && !stage->stalled)
  {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction *current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
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

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Fetch", stage);
    }
  }
  else
    printf("Fetch :\n");
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int decode(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[DRF];
  if (stage->flush == 1)
  {
    strcpy(stage->opcode, " ");
  }

  if (!stage->busy && !stage->stalled)
  {

    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {
      if ((cpu->freeRegisterFlag[stage->rs1]) == 1 && (cpu->freeRegisterFlag[stage->rs2]) == 1)
      {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
      }
      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }
    if (strcmp(stage->opcode, "LOAD") == 0)
    {
      if ((cpu->freeRegisterFlag[stage->rs1]) == 1)
      {
        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
      }
      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }
    if (strcmp(stage->opcode, "ADD") == 0)
    {
      if ((cpu->freeRegisterFlag[stage->rs1]) == 1 && (cpu->freeRegisterFlag[stage->rs2]) == 1)
      {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
      }
      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }

    if (strcmp(stage->opcode, "SUB") == 0)
    {
      if ((cpu->freeRegisterFlag[stage->rs1]) == 1 && (cpu->freeRegisterFlag[stage->rs2]) == 1)
      {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
      }
      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }

    if (strcmp(stage->opcode, "MUL") == 0)
    {
      if ((cpu->freeRegisterFlag[stage->rs1]) == 1 && (cpu->freeRegisterFlag[stage->rs2]) == 1)
      {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
      }
      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }

    if (strcmp(stage->opcode, "ADDL") == 0)
    {
      if ((cpu->freeRegisterFlag[stage->rs1]) == 1)
      {
        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
      }
      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }

    if (strcmp(stage->opcode, "SUBL") == 0)
    {
      if ((cpu->freeRegisterFlag[stage->rs1]) == 1)
      {
        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
      }
      else
      {
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
    }
    if (strcmp(stage->opcode, "BZ") == 0)
    {
      if (cpu->zFlag == 1)
      {
        stage->flush = 1;
        cpu->stage[F].stalled = 1;
        cpu->stage[DRF].stalled = 1;
      }
      else
      {
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
      }
    }
    if (strcmp(stage->opcode, "HALT") == 0)
    {
    }
    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
    }

    /* Copy data from decode latch to execute latch*/

    cpu->stage[QUEUE] = cpu->stage[DRF];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Decode/RF", stage);
    }
  }
  else
    printf("Decode/RF :\n");

  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */

int execute(APEX_CPU *cpu)
{
  CPU_Stage *stage = &cpu->stage[EX];
  if (!stage->busy && !stage->stalled)
  {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {
      if (cpu->freeRegisterFlag[stage->rs1] == 1 && cpu->freeRegisterFlag[stage->rs2])
      {
        stage->buffer = stage->rs2_value + stage->imm;
        if (cpu->freeRegisterFlag[stage->rs1] == 1)
        {
          cpu->regs[stage->buffer] = stage->rs1_value;
        }
      }
    }
    if (strcmp(stage->opcode, "LOAD") == 0)
    {
      if (cpu->freeRegisterFlag[stage->rs1] == 1 && cpu->freeRegisterFlag[stage->rs2])
      {
        stage->buffer = stage->rs1_value + stage->imm;
      }
    }
    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
      stage->buffer = stage->imm;
    }
    if (strcmp(stage->opcode, "BZ") == 0)
    {
    }

    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[RETIRE] = cpu->stage[EX];

    if (ENABLE_DEBUG_MESSAGES)
    {
      print_stage_content("Execute2", stage);
    }
  }
  else
    printf("Execute2 :\n");
  return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */

/**
 * APEX CPU simulation loop
 * @param cpu
 * @return
 */
int APEX_cpu_run(APEX_CPU *cpu, const char *function, int cycles)
{

  if (strcmp(function, "display") == 0)
  {

    /* All the instructions committed, so exit */
    while (cpu->clock != cpu->no_cycles)
    {

      if (ENABLE_DEBUG_MESSAGES)
      {
        printf("--------------------------------\n");
        printf("Clock Cycle #: %d\n", cpu->clock + 1);
        printf("--------------------------------\n");
      }

      execute(cpu);
      decode(cpu);
      fetch(cpu);

      cpu->clock++;

      CPU_Stage *stage1 = &cpu->stage[F];
      CPU_Stage *stage2 = &cpu->stage[DRF];

      if (
          (strcmp(stage2->opcode, "") == 0) &&
          (strcmp(stage1->opcode, "") == 0)

      )
      {
        break;
      }
    }
    printf("(apex) >> Simulation Complete");
    printf("\n");
    printf("=====REGISTER VALUE============\n");
    for (int i = 0; i < 16; i++)
    {
      char *validStr;
      if (cpu->freeRegisterFlag[i] == 1)
      {
        validStr = "Valid";
      }
      else
      {
        validStr = "InValid";
      }

      printf("\n");
      printf(" | Register[%d] | Value=%d | status=%s | \n", i, cpu->regs[i], validStr);
    }
    printf("=======DATA MEMORY===========\n");

    for (int i = 0; i < 99; i++)
    {
      printf(" | MEM[%d] | Value=%d | \n", i, cpu->data_memory[i]);
    }
  }
  else
  {
    /* All the instructions committed, so exit */
    while (cpu->clock == cpu->no_cycles)
    {
      printf("(apex) >> Simulation Complete");

      CPU_Stage *stage1 = &cpu->stage[F];
      CPU_Stage *stage2 = &cpu->stage[DRF];

      if (
          (strcmp(stage2->opcode, "") == 0) &&
          (strcmp(stage1->opcode, "") == 0))
      {
        break;
      }
    }

    cpu->clock++;

    printf("\n");
    printf("=====REGISTER VALUE============\n");
    for (int i = 0; i < 16; i++)
    {
      char *validStr;
      if (cpu->freeRegisterFlag[i] == 1)
      {
        validStr = "Valid";
      }
      else
      {
        validStr = "InValid";
      }

      printf("\n");
      printf(" | Register[%d] | Value=%d | status=%s | \n", i, cpu->regs[i], validStr);
    }
    printf("=======DATA MEMORY===========\n");

    for (int i = 0; i < 99; i++)
    {
      printf(" | MEM[%d] | Value=%d | \n", i, cpu->data_memory[i]);
    }
  }

  return 0;
}