MOVC,R0,#0
MOVC,R3,#3
ADD,R4,R0,R3
MUL,R6,R4,R4
STORE,R6,R0,#4
SUB,R5,R3,R4
MOVC,R8,#32
ADDL,R11,R5,#4
SUBL,R10,R0,#8