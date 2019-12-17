
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"


/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	disp_pos=0;
	for(int i=0;i<80*25;i++){
		disp_str(" ");
	}
	disp_pos=0;

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority =  0;  //this is used when all pro is asleep
	proc_table[1].ticks = proc_table[1].priority =  1;  //Reader A 2
	proc_table[2].ticks = proc_table[2].priority =  1;  //Reader B 3
	proc_table[3].ticks = proc_table[3].priority =  1;  //Reader C 3
	proc_table[4].ticks = proc_table[4].priority =  1;  //Writer D 3
	proc_table[5].ticks = proc_table[5].priority =  1;  //Writer E 4
	proc_table[6].ticks = proc_table[6].priority =  0;  //Printer F

	proc_table[1].need_ticks=2;  proc_table[1].is_reader=1;  
	proc_table[2].need_ticks=3;  proc_table[2].is_reader=1; 
	proc_table[3].need_ticks=3;  proc_table[3].is_reader=1;  
	proc_table[4].need_ticks=3;  proc_table[4].is_reader=0;  
	proc_table[5].need_ticks=4;  proc_table[5].is_reader=0; 

	//rejected is set to 1 if proc doesn't get sem
	//executed_times is used to ensure 'fareness'
	proc_table[1].rejected=0;   proc_table[1].executed_times=0;
	proc_table[2].rejected=0;   proc_table[2].executed_times=0;
	proc_table[3].rejected=0;   proc_table[3].executed_times=0;
	proc_table[4].rejected=0;   proc_table[4].executed_times=0;
	proc_table[5].rejected=0;   proc_table[5].executed_times=0;

	proc_table[1].color=GREEN;	//proc_table[1].has_execute=0;
	proc_table[2].color=BLUE;	//proc_table[2].has_execute=0;
	proc_table[3].color=YELLO;	//proc_table[3].has_execute=0;
	proc_table[4].color=RED;	//proc_table[4].has_execute=0;
	proc_table[5].color=BRIGHT;	//proc_table[5].has_execute=0;
	proc_table[6].color=WHITE;	//proc_table[6].has_execute=0;

	proc_table[0].sleep_time=0;
	proc_table[1].sleep_time=0;
	proc_table[2].sleep_time=0;
	proc_table[3].sleep_time=0;
	proc_table[4].sleep_time=0;
	proc_table[5].sleep_time=0;
	proc_table[6].sleep_time=0;

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	//!!!these are the var that can change
	is_reader_first=0; //change here to switch mode
	count_read=2;  		 //change here to switch numb
	consider_hungry=0;
	hungry_gap=10;

	already_round=0; //hungry val for writer first
	
	hungry_sem.sem_num=0; //used for limit prior process
	op_sem.sem_num=1;

	//semaphore used for read and write
	read_sem.sem_num=count_read;
	write_sem.sem_num=1;
	current_read_num=0;
	
	//change between 0 and 1 every single tick
	f_turn=0; is_writing_now=0; //flag to show if it's writing

	//when all process is asleep, set to 1
	all_asleep=0; if(is_reader_first==1&&consider_hungry==1&&count_read!=1){is_reader_first=0;}

        /* 初始化 8253 PIT */
        out_byte(TIMER_MODE, RATE_GENERATOR);
        out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
        out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

        put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
        enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */

	restart();

	
	while(1){}
}

//====================== my code ==================================

/*used to print special colorful str for process*/
void my_print_color(char* str){
	disp_color_str(str, p_proc_ready->color);
}


/*======================================================================*
						my code
 *======================================================================*/

int delay_time=3000;

//this is called when all readers and writers are asleep
void all_process_asleep()
{
	while(1){
		if(all_asleep==1){
			disp_str("asleep  ");
			milli_delay(2000);
		}
	}
}

void A(){
	//process_sleep(5000);
	while(1){

		if(count_read==1){
			p(&op_sem);
		}

		if(consider_hungry==1&&is_reader_first==1){
			if(p_proc_current->executed_times>hungry_gap){
				p(&hungry_sem);
				p_proc_current->executed_times=0;
			}
		}

		if(is_reader_first==0){
			if(write_sem.sem_num<0){
				continue;
			}
			else{
				goto RUNA;
			}
		}
		else{
			goto RUNA;
		}
RUNA:
		//reader has to gain write sem to prevent writing
		if(write_sem.sem_num==1){
			p(&write_sem);
		}

		p(&read_sem);

		//start reading
		my_print_color("RA S    ");
		//milli_delay(delay_time);
		disp_str("S       ");
		//milli_delay(delay_time);
		int ticks_executed=0; //relevent time ticks
		while(ticks_executed<p_proc_ready->need_ticks){
			my_print_color("RA ing  ");
			f_turn=1;	//it's f's turn, make sure print first
			milli_delay(delay_time);
			ticks_executed++;
		}
		my_print_color("RA F    ");
		//milli_delay(delay_time);
		disp_str("F       ");
		//milli_delay(delay_time);
		
		v(&read_sem);

		if(read_sem.sem_num==count_read){
			//when there's no more readers, i can finally let go
			v(&write_sem);
		}

		if(consider_hungry==1&&is_reader_first==0){
			if(already_round==1&&hungry_sem.sem_num<0){
				v(&hungry_sem);
			}
		}

		if(count_read==1){
			v(&op_sem);
		}
	}
}

void B(){
	//process_sleep(5000);
	while(1){

		if(count_read==1){
			p(&op_sem);
		}
		if(consider_hungry==1&&is_reader_first==1){
			if(p_proc_current->executed_times>hungry_gap){
				p(&hungry_sem);
				p_proc_current->executed_times=0;
			}
		}

		if(is_reader_first==0){
			if(write_sem.sem_num<0){
				continue;
			}
			else{
				goto RUNB;
			}
		}
		else{
			goto RUNB;
		}
RUNB:
		if(write_sem.sem_num==1){
			p(&write_sem);
		}

		p(&read_sem);

		my_print_color("RB S    ");
		//milli_delay(delay_time);
		disp_str("S       ");
		//milli_delay(delay_time);
		int ticks_executed=0;
		while(ticks_executed<p_proc_ready->need_ticks){
			my_print_color("RB ing  ");
			f_turn=1;
			milli_delay(delay_time);
			ticks_executed++;
		}
		my_print_color("RB F    ");
		//milli_delay(delay_time);
		disp_str("F       ");
		//milli_delay(delay_time);
		
		v(&read_sem);

		if(read_sem.sem_num==count_read){
			v(&write_sem);
		}

		if(consider_hungry==1&&is_reader_first==0){
			if(already_round==1&&hungry_sem.sem_num<0){
				v(&hungry_sem);
			}
		}

		if(count_read==1){
			v(&op_sem);
		}
	}
}

void C(){
	//process_sleep(5000);
	while(1){

		if(count_read==1){
			p(&op_sem);
		}
		if(consider_hungry==1&&is_reader_first==1){
			if(p_proc_current->executed_times>hungry_gap){
				p(&hungry_sem);
				p_proc_current->executed_times=0;
			}
		}

		if(is_reader_first==0){
			if(write_sem.sem_num<0){
				continue;
			}
			else{
				goto RUNC;
			}
		}
		else{
			goto RUNC;
		}
RUNC:
		if(write_sem.sem_num==1){
			p(&write_sem);
		}

		p(&read_sem);
		
		my_print_color("RC S    ");
		//milli_delay(delay_time);
		disp_str("S       ");
		//milli_delay(delay_time);
		int ticks_executed=0;
		while(ticks_executed<p_proc_ready->need_ticks){
			my_print_color("RC ing  ");
			f_turn=1;
			milli_delay(delay_time);
			ticks_executed++;
		}
		my_print_color("RC F    ");
		//milli_delay(delay_time);
		disp_str("F       ");
		//milli_delay(delay_time);
		
		v(&read_sem);

		if(read_sem.sem_num==count_read){
			v(&write_sem);
		}

		if(consider_hungry==1&&is_reader_first==0){
			if(already_round==1&&hungry_sem.sem_num<0){
				v(&hungry_sem);
			}
		}

		if(count_read==1){
			v(&op_sem);
		}
	}
}

void D(){
	//process_sleep(5000);
	while(1){

		if(count_read==1){
			p(&op_sem);
		}
		if(consider_hungry==1&&is_reader_first==0){
			//readers are the hungry ones
			//should reject writer's opportunity
			if(already_round==1&&p_proc_current->executed_times>hungry_gap){
				p(&hungry_sem);
				p_proc_current->executed_times=0;
			}
		}

		p(&write_sem);
		
		my_print_color("WD S    ");
		//milli_delay(delay_time);
		disp_str("S       ");
		//milli_delay(delay_time);
		int ticks_executed=0;
		while(ticks_executed<p_proc_ready->need_ticks){
			my_print_color("WD ing  ");
			f_turn=1;
			milli_delay(delay_time);
			ticks_executed++;
		}
		my_print_color("WD F    ");
		//milli_delay(delay_time);
		disp_str("F       ");
		//milli_delay(delay_time);

		v(&write_sem);

		if(consider_hungry==1&&is_reader_first==1){
			//if reader first, let the hungry process to release sem
			if(hungry_sem.sem_num<0){
				v(&hungry_sem);
			}
		}

		if(count_read==1){
			v(&op_sem);
		}
	}
}

void E(){
	//process_sleep(5000);
	while(1){

		if(count_read==1){
			p(&op_sem);
		}
		if(consider_hungry==1&&is_reader_first==0){
			//readers are the hungry ones
			//should reject writer's opportunity
			if(already_round==1&&p_proc_current->executed_times>hungry_gap){
				p(&hungry_sem);
				p_proc_current->executed_times=0;
			}
		}

		p(&write_sem);

		my_print_color("WE S    ");
		//milli_delay(delay_time);
		disp_str("S       ");
		//milli_delay(delay_time);
		int ticks_executed=0;
		while(ticks_executed<p_proc_ready->need_ticks){
			my_print_color("WE ing  ");
			f_turn=1;
			milli_delay(delay_time);
			ticks_executed++;
		}
		my_print_color("WE F    ");
		//milli_delay(delay_time);
		disp_str("F       ");
		//milli_delay(delay_time);

		v(&write_sem);

		if(consider_hungry==1&&is_reader_first==1){
			//if reader first, let the hungry process to release sem
			if(hungry_sem.sem_num<0){
				v(&hungry_sem);
			}
		}

		if(count_read==1){
			v(&op_sem);
		}
	}
}
void F(){
	while(1){
		if(f_turn){
			if(all_asleep==1){
				//no rules for f to print when all asleep
				continue;
			}
			if(p_proc_current->is_reader==1){
				//obviously it's reader operating
				current_read_num=count_read-(read_sem.sem_num<0?0:read_sem.sem_num);
				print_str("R ");
				disp_int(current_read_num);
				print_str("   ");
			}
			else{
				print_str("W       ");
			}
			f_turn=0;
		}
	}
}
