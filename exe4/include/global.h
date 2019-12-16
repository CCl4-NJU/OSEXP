
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            global.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* EXTERN is defined as extern except in global.c */
#ifdef	GLOBAL_VARIABLES_HERE
#undef	EXTERN
#define	EXTERN
#endif

EXTERN	int		ticks;

EXTERN	int		disp_pos;
EXTERN	u8		gdt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	DESCRIPTOR	gdt[GDT_SIZE];
EXTERN	u8		idt_ptr[6];	// 0~15:Limit  16~47:Base
EXTERN	GATE		idt[IDT_SIZE];


EXTERN	u32		    k_reenter;

EXTERN	TSS		    tss;
EXTERN	PROCESS*	p_proc_ready;

extern	PROCESS		proc_table[];
extern	char		task_stack[];
extern  TASK        task_table[];
extern	irq_handler	irq_table[];

//=========================== my code ===============================

SEMAPHORE read_sem;  //sem of readers
SEMAPHORE write_sem; //sem of writers
int reader_applying_write_sem;

int count_read;  //max reader number
int current_read_num;

PROCESS* p_proc_current;

int f_turn;

int all_asleep;  

int is_reader_first;

int is_writing_now;

int consider_hungry;
int hungry_gap; //hungry gap value, it's the subtraction between
//the max executed_times and min executed_times
int already_round;
SEMAPHORE hungry_sem;

SEMAPHORE op_sem;