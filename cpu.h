#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_
enum
{
  F,
  DRF,
  //QUEUE,
  INT_FU1,
  INT_FU2,
  MUL_FU1,
  MUL_FU2,
  MUL_FU3,
  MEM,
  NUM_STAGES,

  RETIRE

};

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
  char opcode[128]; // Operation Code
  int rd;           // Destination Register Address
  int rs1;          // Source-1 Register Address
  int rs2;
  int rs3;// Source-2 Register Address
  int imm;          // Literal Value
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
  int pc;           // Program Counter
  char opcode[128]; // Operation Code
  int rs1;          // Source-1 Register Address
  int rs2;
  int rs3;// Source-2 Register Address
  int rd;           // Destination Register Address
  int imm;          // Literal Value
  int rs1_value;    // Source-1 Register Value
  int rs2_value;
    int rs3_value; // Source-2 Register Value
  int buffer;       // Latch to hold some value
  int mem_address;  // Computed Memory Address
  int busy;         // Flag to indicate, stage is performing some action
  int stalled;      // Flag to indicate, stage is stalled
  int flush;        // Flag to flush when branch is taken
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
  /* Clock cycles elapsed */
  int clock;

  /* Current program counter */
  int pc;

  int no_cycles;
  /* Integer register file */
  int regs[32];

  int freeRegisterFlag[32];

  /* Array of 5 CPU_stage */
  CPU_Stage stage[10];

  /* Code Memory where instructions are stored */
  APEX_Instruction *code_memory;
  int code_memory_size;

  /* Data Memory */
  int data_memory[4096];

  /* Some stats */
  int ins_completed;

  /* IQ data*/

  int zFlag;

  int memFetchStore;
  int flushBranchTakenData;
  int newPCBranchTaken;

  int haltEncountered;
  int haltRetiredFromROB;

} APEX_CPU;

struct QueueEntry
{
  char opcode[128]; // Operation Code

  int rs1;
  int rs1_ready;
  int rs1_value;

  int rs2;
  int rs2_ready;
  int rs2_value;

  int rd;
  int imm;

  int pc;                   // Program Counter
  int isAvailableStatusBit; //indicates if this IQ entry is allocated or free

  int IQFront;
  int IQRear;
} QueueEntry;

struct QueueEntry IQ[8];

struct LSQ
{
  struct QueueEntry LSQEntry;
  int LOADSTOREBit;

} LSQ[6];

struct functionalUnits
{
  char opcode[128]; // Operation Code

  int rs1;
  int rs1_value;

  int rs2;
  int rs2_value;

  int buffer;      //store the calculated value.
  int mem_address; //store the calculated value.

  int rd;
  int imm;

  int pc;
  int dummyEntry;
} functionalUnits;

struct prf{
    int valid;
    int ready;
    int value;
    int latest;
    int arf_val;
}prf[24];
int pcount;
//int memcycles;
//struct ROB
//{
//
//  int ROBHeadPointer;
//  int ROBTailPointer;
//
// struct APEX_Instruction ROBAPEXInstruction;
//} ROB[12];

APEX_Instruction *create_code_memory(const char *filename, int *size);

APEX_CPU *APEX_cpu_init(const char *filename);

int APEX_cpu_run(APEX_CPU *cpu, const char *function, int cycles);

void APEX_cpu_stop(APEX_CPU *cpu);

APEX_Instruction ROB[8];

int RF,RR;

int fetch(APEX_CPU *cpu);

int fun(struct prf prf[]);

int decode(APEX_CPU *cpu);

int execute(APEX_CPU *cpu);

int memory2(APEX_CPU *cpu);

void freephyreg(struct prf prf[], int free);

#endif
