#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

#define ENABLE_DEBUG_MESSAGES 1

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

  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->freeRegisterFlag, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

    for(int i=0; i<24;i++)
    {
        prf[i].valid=1;
        prf[i].value=-1;
        prf[i].latest=0;

    }
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


  for (int i = 1; i < NUM_STAGES; ++i)
  {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}
int memcycles=0;
int RF=0,RR=0;
void printROB(){
    for (int i = 0; i < 12; ++i) {

//            if(strcmp(ROB[i].opcode,"")==0){
//                break;
//            }
            if (strcmp(ROB[i].opcode, "STR") == 0) {
                printf("%s,R%d,R%d,#%d\n", ROB[i].opcode, ROB[i].rs1, ROB[i].rs2, ROB[i].imm);
            }
                if (strcmp(ROB[i].opcode, "SUBL") == 0 ||
                       strcmp(ROB[i].opcode, "ADDL") == 0 ||
                       strcmp(ROB[i].opcode, "LOAD") == 0
                    ) {
                printf("%s,R%d,R%d,#%d\n", ROB[i].opcode, ROB[i].rd, ROB[i].rs1, ROB[i].imm);
            } if (strcmp(ROB[i].opcode, "MOVC") == 0) {
                printf("%s,R%d,R#%d\n", ROB[i].opcode, ROB[i].rd, ROB[i].imm);
            } if (strcmp(ROB[i].opcode, "BZ") == 0 ||
                       strcmp(ROB[i].opcode, "BNZ") == 0
                    ) {
                printf("%s,#%d\n", ROB[i].opcode, ROB[i].imm);
            }  if(strcmp(ROB[i].opcode, "ADD") == 0 ||
                        strcmp(ROB[i].opcode, "MUL") == 0 ||
                        strcmp(ROB[i].opcode, "LDR") == 0 ||
                        strcmp(ROB[i].opcode, "SUB") == 0

            ){
                printf("%s,R%d,R%d,R%d\n", ROB[i].opcode, ROB[i].rd, ROB[i].rs1, ROB[i].rs2);
            }
    }
}
void addROB(struct APEX_Instruction APEX_Instruction)
{
    ROB[RR]=APEX_Instruction;
    RR++;
}
void delROB(){

    for(int i=0; i<12; i++)
    {
        ROB[i]=ROB[i+1];

    }
}

int fun(struct prf prf[]) {
    int pcount = 0;
    for (int i = 0; i < 24; ++i) {
        if (prf[i].valid == 1 && prf[i].value == -1)
            break;
        else pcount++;

    }
    return pcount;
}

void freephyreg(struct prf prf[], int free){
    prf[free].latest=0;
    prf[free].valid=1;
    prf[free].arf_val=-1;
    prf[free].value=-1;
}

void APEX_cpu_stop(APEX_CPU *cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

int get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void print_instruction(CPU_Stage *stage)
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
    if (strcmp(stage->opcode, "STR") == 0)
    {
        printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rs1, stage->rs2, stage->rs3);
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

static void print_stage_content(char *name, CPU_Stage *stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

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

int decode(APEX_CPU *cpu)
{
    int intcounter=0,mulcounter=0;
  CPU_Stage *stage = &cpu->stage[DRF];
  APEX_Instruction apex;
    strcpy(apex.opcode,stage->opcode);
    apex.rd=stage->rd;
    apex.rs1=stage->rs1;
    apex.rs2=stage->rs2;
    apex.rs3=stage->rs3;
    apex.imm=stage->imm;
    addROB(apex);

  if (stage->flush == 1)
  {
    strcpy(stage->opcode, " ");
  }

  if (!stage->busy && !stage->stalled)
  {

    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {

        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
          int rs1f=0,rs2f=0;
          for (int i = 0; i <24 ; ++i)
          {
              if(stage->rs1==prf[i].value){
                  rs1f=-1;
                  break;
              }
          }
          for (int i = 0; i <24 ; ++i)
          {
              if(stage->rs2==prf[i].value){
                  rs2f=-1;
                  break;
              }
          }
          if(rs1f!=-1) {
              pcount = fun(prf);
              prf[pcount].value = stage->rs1;
              prf[pcount].valid = 0;
          }
          if(rs2f!=-1) {
              pcount = fun(prf);
              prf[pcount].value = stage->rs2;
              prf[pcount].valid = 0;
          }
    }
    if (strcmp(stage->opcode, "LOAD") == 0)
    {

        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
        int rs1f=0;
        for (int i = 0; i <24 ; ++i)
        {
            if(stage->rs1==prf[i].value){
                rs1f=-1;
                break;
            }
        }

        if(rs1f!=-1) {
            pcount = fun(prf);
            prf[pcount].value = stage->rs1;
            prf[pcount].valid = 0;
        }
        pcount=fun(prf);
        prf[pcount].value=stage->rd;
        prf[pcount].valid=0;

    }
    if (strcmp(stage->opcode, "ADD") == 0)
    {


        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        int rs1f=0,rs2f=0;
        for (int i = 0; i <24 ; ++i)
        {
            if(stage->rs1==prf[i].value){
                rs1f=-1;
                break;
            }
        }
        for (int i = 0; i <24 ; ++i)
        {
            if(stage->rs2==prf[i].value){
                rs2f=-1;
                break;
            }
        }
        if(rs1f!=-1) {
            pcount = fun(prf);
            prf[pcount].value = stage->rs1;
            prf[pcount].valid = 0;
        }
        if(rs2f!=-1) {
            pcount = fun(prf);
            prf[pcount].value = stage->rs2;
            prf[pcount].valid = 0;
        }
          pcount=fun(prf);
          prf[pcount].value=stage->rd;
          prf[pcount].valid=0;
    }
    if (strcmp(stage->opcode, "SUB") == 0)
    {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;

          int rs1f=0,rs2f=0;
          for (int i = 0; i <24 ; ++i)
          {
              if(stage->rs1==prf[i].value){
                  rs1f=-1;
                  break;
              }
          }
          for (int i = 0; i <24 ; ++i)
          {
              if(stage->rs2==prf[i].value){
                  rs2f=-1;
                  break;
              }
          }
          if(rs1f!=-1) {
              pcount = fun(prf);
              prf[pcount].value = stage->rs1;
              prf[pcount].valid = 0;
          }
          if(rs2f!=-1) {
              pcount = fun(prf);
              prf[pcount].value = stage->rs2;
              prf[pcount].valid = 0;
          }
          pcount=fun(prf);
          prf[pcount].value=stage->rd;
          prf[pcount].valid=0;

    }
    if (strcmp(stage->opcode, "MUL") == 0)
    {

        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
          int rs1f=0,rs2f=0;
          for (int i = 0; i <24 ; ++i)
          {
              if(stage->rs1==prf[i].value){
                  rs1f=-1;
                  break;
              }
          }
          for (int i = 0; i <24 ; ++i)
          {
              if(stage->rs2==prf[i].value){
                  rs2f=-1;
                  break;
              }
          }
          if(rs1f!=-1) {
              pcount = fun(prf);
              prf[pcount].value = stage->rs1;
              prf[pcount].valid = 0;
          }
          if(rs2f!=-1) {
              pcount = fun(prf);
              prf[pcount].value = stage->rs2;
              prf[pcount].valid = 0;
          }
          pcount=fun(prf);
          prf[pcount].value=stage->rd;
          prf[pcount].valid=0;

    }
    if (strcmp(stage->opcode, "ADDL") == 0)
    {

        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
          int rs1f=0;
          for (int i = 0; i <24 ; ++i)
          {
              if(stage->rs1==prf[i].value){
                  rs1f=-1;
                  break;
              }
          }

          if(rs1f!=-1) {
              pcount = fun(prf);
              prf[pcount].value = stage->rs1;
              prf[pcount].valid = 0;
          }

          pcount=fun(prf);
          prf[pcount].value=stage->rd;
          prf[pcount].valid=0;

    }
    if (strcmp(stage->opcode, "SUBL") == 0)
    {
        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->stage[F].stalled = 0;
        cpu->stage[DRF].stalled = 0;
        int rs1f=0;
        for (int i = 0; i <24 ; ++i)
        {
            if(stage->rs1==prf[i].value){
                rs1f=-1;
                break;
            }
        }

        if(rs1f!=-1) {
            pcount = fun(prf);
            prf[pcount].value = stage->rs1;
            prf[pcount].valid = 0;
        }

        pcount=fun(prf);
        prf[pcount].value=stage->rd;
        prf[pcount].valid=0;

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
    if (strcmp(stage->opcode, "MOVC") == 0)
    {
        pcount=fun(prf);
        prf[pcount].value=stage->rd;
        prf[pcount].valid=0;
    }



    if (
              strcmp(stage->opcode, "LOAD") == 0 ||
              strcmp(stage->opcode, "LDR")  == 0 ||
              strcmp(stage->opcode, "ADD")  == 0 ||
              strcmp(stage->opcode, "SUB")  == 0 ||
              strcmp(stage->opcode, "STORE")== 0 ||
              strcmp(stage->opcode, "STR")  == 0 ||
              strcmp(stage->opcode, "MOVC") == 0 ||
              strcmp(stage->opcode, "ADDL") == 0 ||
              strcmp(stage->opcode, "SUBL") == 0){
          cpu->stage[INT_FU1]=cpu->stage[DRF];
          intcounter++;

      }
      if(strcmp(stage->opcode, "MUL") == 0) {
          cpu->stage[MUL_FU1]=cpu->stage[DRF];
          mulcounter++;
      }
    if (ENABLE_DEBUG_MESSAGES)
    {
        printf("---------------------------------RAT-------------------------------------\n");
        for (int j = 0; j < 24; ++j) {
            if(prf[j].valid!=1)
                printf("|R[%d] = P%d & valid = %d ARF_VAL=%d Latest=%d|\n",prf[j].value,j, prf[j].valid,prf[j].arf_val,prf[j].latest);
        }
        printf("---------------------------------RAT-------------------------------------\n");

        printf("---------------------------------ROB-------------------------------------\n");
            printROB();
        printf("---------------------------------ROB-------------------------------------\n");

        print_stage_content("Decode/RF", stage);

    }
    if(intcounter==0)
        strcpy(cpu->stage[INT_FU1].opcode,"");
      if(mulcounter==0)
          strcpy(cpu->stage[MUL_FU1].opcode,"");
  }
  return 0;
}

int intfu1(APEX_CPU *cpu)
{
    CPU_Stage *stage = &cpu->stage[INT_FU1];
    int frd=0,frs1=0,frs2=0,free=0,frs3=0;
    if(strcmp(stage->opcode,"MOVC")==0){

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==0){
                break;
            }
            else frd++;
        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==1){
                break;
            }
            else free++;
        }
        freephyreg(prf,free);
        prf[frd].arf_val=stage->imm;
        prf[frd].latest=1;

    }
    if(strcmp(stage->opcode,"ADD")==0){

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs1 && prf[i].latest==1){
                break;
            }
            else frs1++;
        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs2 && prf[i].latest==1){
                break;
            }
            else frs2++;

        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==0){
                break;
            }
            else frd++;
        }

            stage->rs1_value=prf[frs1].arf_val;
            stage->rs2_value=prf[frs2].arf_val;
            stage->buffer=stage->rs1_value+stage->rs2_value;

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==1){
                break;
            }
            else free++;
        }
            freephyreg(prf,free);
            prf[frd].arf_val=stage->buffer;
            prf[frd].latest=1;

    }
    if(strcmp(stage->opcode,"SUB")==0){

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs1 && prf[i].latest==1){
                break;
            }
            else frs1++;
        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs2 && prf[i].latest==1){
                break;
            }
            else frs2++;

        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==0){
                break;
            }
            else frd++;
        }

        stage->rs1_value=prf[frs1].arf_val;
        stage->rs2_value=prf[frs2].arf_val;
        stage->buffer=stage->rs1_value-stage->rs2_value;

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==1){
                break;
            }
            else free++;
        }
        freephyreg(prf,free);
        prf[frd].arf_val=stage->buffer;
        prf[frd].latest=1;


    }
    if(strcmp(stage->opcode,"STORE")==0){

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs1 && prf[i].latest==1){
                break;
            }
            else frs1++;
        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs2 && prf[i].latest==1){
                break;
            }
            else frs2++;

        }

        stage->rs1_value=prf[frs1].arf_val;
        stage->rs2_value=prf[frs2].arf_val;
        stage->buffer=stage->rs2_value+stage->imm;

    }
    if(strcmp(stage->opcode,"SUBL")==0){

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs1 && prf[i].latest==1){
                break;
            }
            else frs1++;
        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==0){
                break;
            }
            else frd++;
        }

        stage->rs1_value=prf[frs1].arf_val;
        stage->buffer=stage->rs1_value-stage->imm;
        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==1){
                break;
            }
            else free++;
        }
        freephyreg(prf,free);
        prf[frd].arf_val=stage->buffer;
        prf[frd].latest=1;

        }
    if(strcmp(stage->opcode,"ADDL")==0){

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs1 && prf[i].latest==1){
                break;
            }
            else frs1++;
        }



        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==0){
                break;
            }
            else frd++;
        }

        stage->rs1_value=prf[frs1].arf_val;
        stage->buffer=stage->rs1_value + stage->imm;
        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==1){
                break;
            }
            else free++;
        }
        freephyreg(prf,free);
        prf[frd].arf_val=stage->buffer;
        prf[frd].latest=1;

    }
    if(strcmp(stage->opcode,"STR")==0){

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs1 && prf[i].latest==1){
                break;
            }
            else frs1++;
        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs2 && prf[i].latest==1){
                break;
            }
            else frs2++;

        }
        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs3 && prf[i].latest==1){
                break;
            }
            else frs3++;

        }

        stage->rs1_value=prf[frs1].arf_val;
        stage->rs2_value=prf[frs2].arf_val;
        stage->rs3_value=prf[frs3].arf_val;
        stage->buffer=stage->rs2_value + stage->rs3_value;

    }
    if(strcmp(stage->opcode,"LOAD")==0){

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs1 && prf[i].latest==1){
                break;
            }
            else frs1++;
        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==0){
                break;
            }
            else frd++;
        }

        stage->rs1_value=prf[frs1].arf_val;
        stage->buffer=stage->rs1_value + stage->imm;
        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==1){
                break;
            }
            else free++;
        }
        freephyreg(prf,free);
        prf[frd].arf_val=stage->buffer;
        prf[frd].latest=1;

    }
    if(strcmp(stage->opcode,"LDR")==0){

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs1 && prf[i].latest==1){
                break;
            }
            else frs1++;
        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs2 && prf[i].latest==1){
                break;
            }
            else frs2++;

        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==0){
                break;
            }
            else frd++;
        }

        stage->rs1_value=prf[frs1].arf_val;
        stage->rs2_value=prf[frs2].arf_val;
        stage->buffer=stage->rs1_value+stage->rs2_value;

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==1){
                break;
            }
            else free++;
        }
        freephyreg(prf,free);
        prf[frd].arf_val=stage->buffer;
        prf[frd].latest=1;

    }
    cpu->stage[INT_FU2]=cpu->stage[INT_FU1];
    if (ENABLE_DEBUG_MESSAGES)
    {
        print_stage_content("Integer FU1", stage);
    }


    return 0;
}

int intfu2(APEX_CPU *cpu)
{
    int strcounter=0, othercounter=0;
    CPU_Stage *stage = &cpu->stage[INT_FU2];
        if(strcmp(stage->opcode,"STORE")==0  ||
           strcmp(stage->opcode,"LOAD") ==0  ||
           strcmp(stage->opcode,"STR")  ==0  ||
           strcmp(stage->opcode,"LDR")  ==0
        ){
            cpu->stage[MEM]=cpu->stage[INT_FU2];
            strcounter++;
        }
        else {
            cpu->stage[RETIRE] = cpu->stage[INT_FU2];
            othercounter++;
        }
    if (ENABLE_DEBUG_MESSAGES)
    {
        print_stage_content("Integer FU2", stage);
    }
    if(strcounter==0)
        strcpy(cpu->stage[MEM].opcode,"");
    if(othercounter==0)
        strcpy(cpu->stage[RETIRE].opcode,"");

    return 0;
}

int mulfu1(APEX_CPU *cpu)
{
    CPU_Stage *stage = &cpu->stage[MUL_FU1];

    //if(!stage->stalled)
    //cpu->stage[RETIRE] = cpu->stage[EX];
    cpu->stage[MUL_FU2]=cpu->stage[MUL_FU1];
    if (ENABLE_DEBUG_MESSAGES)
    {
        print_stage_content("MUL FU1", stage);
    }
    return 0;
}

int mulfu2(APEX_CPU *cpu)
{
    CPU_Stage *stage = &cpu->stage[MUL_FU2];

    //if(!stage->stalled)
    //cpu->stage[RETIRE] = cpu->stage[EX];
    cpu->stage[MUL_FU3]=cpu->stage[MUL_FU2];
    if (ENABLE_DEBUG_MESSAGES)
    {
        print_stage_content("MUL FU2", stage);
    }


    return 0;
}

int mulfu3(APEX_CPU *cpu){
    CPU_Stage *stage = &cpu->stage[MUL_FU3];
    int frd=0,frs1=0,frs2=0,free=0;
    if(strcmp(stage->opcode,"MUL")==0){

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs1 && prf[i].latest==1){
                break;
            }
            else frs1++;
        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rs2 && prf[i].latest==1){
                break;
            }
            else frs2++;

        }

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==0){
                break;
            }
            else frd++;
        }

        stage->rs1_value=prf[frs1].arf_val;
        stage->rs2_value=prf[frs2].arf_val;
        stage->buffer=stage->rs1_value*stage->rs2_value;

        for (int i = 0; i <24 ; ++i) {
            if(prf[i].value==stage->rd && prf[i].latest==1){
                break;
            }
            else free++;
        }
        freephyreg(prf,free);
        prf[frd].arf_val=stage->buffer;
        prf[frd].latest=1;

    }
    cpu->stage[RETIRE]=cpu->stage[MUL_FU3];
    if (ENABLE_DEBUG_MESSAGES)
    {
        print_stage_content("MUL FU3", stage);
    }
    return 0;

}

int mem(APEX_CPU *cpu){
    CPU_Stage *stage = &cpu->stage[MEM];
    int frs1=0,frd=0,free=0;

        if (strcmp(stage->opcode, "STORE") == 0 || strcmp(stage->opcode, "STR")) {

            for (int i = 0; i < 24; ++i) {
                if (prf[i].value == stage->rs1 && prf[i].latest == 1) {
                    break;
                } else frs1++;
            }
            stage->rs1_value = prf[frs1].arf_val;
            cpu->regs[stage->buffer] = stage->rs1_value;
            freephyreg(prf, frs1);
        }

        if (strcmp(stage->opcode, "LDR") == 0 || strcmp(stage->opcode, "LOAD") == 0) {

            for (int i = 0; i < 24; ++i) {
                if (prf[i].value == stage->rd && prf[i].latest == 0) {
                    break;
                } else frd++;
            }

            stage->rs1_value = prf[frs1].arf_val;
            stage->buffer = stage->rs1_value + stage->imm;
            for (int i = 0; i < 24; ++i) {
                if (prf[i].value == stage->rd && prf[i].latest == 1) {
                    break;
                } else free++;
            }
            freephyreg(prf, free);
            prf[frd].arf_val = stage->buffer;
            prf[frd].latest = 1;
        }

        cpu->stage[RETIRE] = cpu->stage[MEM];

        if (ENABLE_DEBUG_MESSAGES)
    {
        print_stage_content("Memmory", stage);
    }

    return 0;
}

int retire(APEX_CPU *cpu){
    CPU_Stage *stage = &cpu->stage[RETIRE];
    delROB();
    if (ENABLE_DEBUG_MESSAGES)
    {
        print_stage_content("Retired", stage);
    }

    cpu->ins_completed++;
    return 0;
}



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
      retire(cpu);
      mem(cpu);
      mulfu3(cpu);
      mulfu2(cpu);
      mulfu1(cpu);
      intfu2(cpu);
      intfu1(cpu);
      decode(cpu);
      fetch(cpu);

      cpu->clock++;

      CPU_Stage *stage1 = &cpu->stage[F];
      CPU_Stage *stage2 = &cpu->stage[DRF];
      CPU_Stage *stage3 = &cpu->stage[INT_FU1];
      CPU_Stage *stage4 = &cpu->stage[INT_FU2];
      CPU_Stage *stage5 = &cpu->stage[MUL_FU1];
      CPU_Stage *stage6 = &cpu->stage[MUL_FU2];
      CPU_Stage *stage7 = &cpu->stage[MUL_FU3];
      CPU_Stage *stage8 = &cpu->stage[MEM];
      CPU_Stage *stage9 = &cpu->stage[RETIRE];


      if (
          (strcmp(stage2->opcode, "") == 0) &&
          (strcmp(stage1->opcode, "") == 0) &&
          (strcmp(stage3->opcode, "") == 0) &&
          (strcmp(stage4->opcode, "") == 0) &&
                  (strcmp(stage6->opcode, "") == 0) &&
                  (strcmp(stage7->opcode, "") == 0) &&
                  (strcmp(stage8->opcode, "") == 0) &&
                  (strcmp(stage9->opcode, "") == 0) &&
          (strcmp(stage5->opcode, "") == 0)

      )
      {
        break;
      }
    }
    printf("(apex) >> Simulation Complete");
    printf("\n");

    printf("++++++++++++++RAT++++++++++++++++\n");
      for (int j = 0; j < 24; ++j) {
          if(prf[j].valid!=1)
          printf("|R[%d] = P%d & valid = %d ARF_VAL=%d Latest=%d|\n",prf[j].value,j, prf[j].valid,prf[j].arf_val,prf[j].latest);
      }
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
