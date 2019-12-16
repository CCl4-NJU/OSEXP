
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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

//if it's writer_first, in order to start from reader
//it's necessary to make writer 'invisible' first
int for_writer_first=1;

/*======================================================================*
                             schedule
			this is my schedule method, cover the old one
 *======================================================================*/
PUBLIC void schedule(){
	disable_irq(CLOCK_IRQ);
	int all_executed=1;
	int call_sleep_p=1; //in case all readers and writers are asleep
	int sleep_num=0;
	//first,need to decrease the ticks of sleeping process
	for(int i=1;i<6;i++){
		if(proc_table[i].sleep_time>=0){
			if(proc_table[i].sleep_time==0){
				all_asleep=0;
				call_sleep_p=0;
				continue;
			}
			PROCESS* tp = &proc_table[i];
			tp->sleep_time = tp->sleep_time-1;
			sleep_num++;
		}
		else{
			all_asleep=0;
			call_sleep_p=0;
		}
	}

	if(call_sleep_p==1){ //only can choose the sleeping processing process
		if(sleep_num==5){
			all_asleep=1;
		}
		p_proc_ready=&proc_table[0];
		enable_irq(CLOCK_IRQ);
		return;
	}
	
	//reader first schedule
	if(is_reader_first==1){
		//check if it's f's turn to print
		if(f_turn){
			p_proc_ready=&proc_table[6];
			enable_irq(CLOCK_IRQ);
			return;
		}

		PROCESS* ready[6];
		int ready_num=0;

		//select reader first because of the rule
		for(int i=1;i<6;i++){
			//the chosen process mustn't be rejected
			if(proc_table[i].rejected==0&&proc_table[i].is_reader==1){
				if(proc_table[i].sleep_time>0){
					//is asleep, dont considered
					PROCESS* tp = &proc_table[i];
					tp->sleep_time = tp->sleep_time-1;
					continue;
				}
				//if(proc_table[i].has_execute==1){
				//	continue;
				//}
				ready[ready_num]=&proc_table[i];
				ready_num++;
			}
		}

		if(ready_num>0){ //if there's anything chosen
			all_executed=0;
			PROCESS* chosen_to_ready=ready[0];
			
			for(int i=1;i<ready_num;i++){
				if(ready[i]->executed_times < chosen_to_ready->executed_times){
					chosen_to_ready=ready[i];
				}
			}
			p_proc_ready=chosen_to_ready;
			chosen_to_ready->executed_times++; //plus one to decrease the choosing right
		}
		else{
			//haven't chosen any readers(not really possible in reader first)
			for(int i=1;i<6;i++){
				if(proc_table[i].rejected==0&&proc_table[i].is_reader==0){
					if(proc_table[i].sleep_time>0){
						//is asleep, dont considered
						//PROCESS* tp = &proc_table[i];
						//tp->sleep_time = tp->sleep_time-1;
						continue;
					}
					//if(proc_table[i].has_execute==1){
					//	continue;
					//}
					ready[ready_num]=&proc_table[i];
					ready_num++;
				}
			}

			if(ready_num>0){
				all_executed=0;
				PROCESS* chosen_to_ready=ready[0];
				for(int i=1;i<ready_num;i++){
					if(ready[i]->executed_times < chosen_to_ready->executed_times){
						chosen_to_ready=ready[i];
					}
				}
				p_proc_ready=chosen_to_ready;
				chosen_to_ready->executed_times++;
			}
		}
		p_proc_current=p_proc_ready;
	}
	else{
		if(f_turn){
			p_proc_ready=&proc_table[6];
			enable_irq(CLOCK_IRQ);
			return;
		}

		PROCESS* ready[6];
		int ready_num=0;
/*
//======================= hungry schedule part ==============================
		if(consider_hungry!=1){
			goto NORMAL_SCHED;
		}
		else{

			if(already_round!=1){
				goto NORMAL_SCHED;
			}
			else{
				for(int i=1;i<=for_writer_first;i++){
					if(proc_table[i].rejected==0){
						if(proc_table[i].sleep_time>0){
							//is asleep, dont considered
							//PROCESS* tp = &proc_table[i];
							//tp->sleep_time = tp->sleep_time-1;
							continue;
						}
						//if(proc_table[i].has_execute==1){
						//	continue;
						//}
						ready[ready_num]=&proc_table[i];
						ready_num++;
					}
				}
				if(ready_num<=1){
					if(ready_num==0){
						p_proc_ready=&proc_table[0];
					}
					else{
						//indicates writer is still writing
						p_proc_current=p_proc_ready=&ready[0];
						p_proc_ready->executed_times++;
					}
				}
				else{
					//indicates all five pros are ready, or only readers.
					int max_executed_times=1;
					int min_executed_times=1;
					PROCESS* hungry_proc;
					for(int j=0;j<6;j++){
						if(proc_table[j].executed_times>max_executed_times){
							max_executed_times=proc_table[j].executed_times;
						}
						if(proc_table[j].executed_times<=min_executed_times){
							max_executed_times=proc_table[j].executed_times;
							hungry_proc=&proc_table[j];
						}
					}
					if(hungry_gap<=max_executed_times-min_executed_times){
						p_proc_ready=p_proc_current=hungry_proc;
						p_proc_ready->executed_times++;
					}
					else{
						PROCESS* n_ready[6];
						int n_ready_num=0;
						for(int k=0;k<ready_num;k++){
							if(ready[k]->is_reader==0){
								n_ready[n_ready_num]=ready[k];
								n_ready_num++;
							}
						}
						if(n_ready_num>0){
							PROCESS* n_chosen_to_ready=n_ready[0];
				
							for(int jj=1;jj<ready_num;jj++){
								if(ready[jj]->executed_times < n_chosen_to_ready->executed_times){
									n_chosen_to_ready=ready[jj];
								}
							}
							p_proc_current=p_proc_ready=n_chosen_to_ready;
							n_chosen_to_ready->executed_times++;
						}
						else{
							goto WF_CHOOSE_READER;
						}
					}
				}
			}
		}
		*/
//===========================================================================
NORMAL_SCHED:
		//for_writer_first is used for 'hiding' the existence of writer first
		//so that readers can be chosen to execute in the beginning
		for(int i=1;i<=for_writer_first;i++){
			if(proc_table[i].rejected==0&&proc_table[i].is_reader==0){
				if(proc_table[i].sleep_time>0){
					//is asleep, dont considered
					//PROCESS* tp = &proc_table[i];
					//tp->sleep_time = tp->sleep_time-1;
					continue;
				}
				//if(proc_table[i].has_execute==1){
				//	continue;
				//}
				ready[ready_num]=&proc_table[i];
				ready_num++;
			}
		}

		if(ready_num>0){
			all_executed=0;
			PROCESS* chosen_to_ready=ready[0];
			
			for(int i=1;i<ready_num;i++){
				if(ready[i]->executed_times < chosen_to_ready->executed_times){
					chosen_to_ready=ready[i];
				}
			}
			p_proc_ready=chosen_to_ready;
			chosen_to_ready->executed_times++;
		}
		else{
WF_CHOOSE_READER:
			for(int i=1;i<=for_writer_first;i++){
				if(proc_table[i].rejected==0&&proc_table[i].is_reader==1){
					if(proc_table[i].sleep_time>0){
						//is asleep, dont considered
						//PROCESS* tp = &proc_table[i];
						//tp->sleep_time = tp->sleep_time-1;
						continue;
					}
					//if(proc_table[i].has_execute==1){
					//	continue;
					//}
					ready[ready_num]=&proc_table[i];
					ready_num++;
				}
			}

			if(ready_num>0){
				all_executed=0;
				PROCESS* chosen_to_ready=ready[0];
				int k=0;
				for(int i=1;i<ready_num;i++){
					if(ready[i]->executed_times < chosen_to_ready->executed_times){
						chosen_to_ready=ready[i];
					}
				}
				p_proc_ready=chosen_to_ready;
				chosen_to_ready->executed_times++;
				
			}

		}

		if(for_writer_first<5){//increase the var, when it's >=4, becomes real writer first
			for_writer_first++;
		}
		else{
			already_round=1;
		}
		p_proc_current=p_proc_ready;
	}

	enable_irq(CLOCK_IRQ);
}


/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

//======================== my code ======================================

/* p.s. since it's system call, we need to ensure atomic op */
/* thus, it's necessary to close irq first */

PUBLIC void sys_process_sleep(int milli_seconds){
	disable_irq(CLOCK_IRQ);
	int sleep_ticks=milli_seconds*HZ/1000+1;
	p_proc_ready->sleep_time=sleep_ticks;
	enable_irq(CLOCK_IRQ);
	schedule();
}

PUBLIC void sys_print_str(char* str){
		disable_irq(CLOCK_IRQ);
		disp_str(str);
		enable_irq(CLOCK_IRQ);
}

PUBLIC void sys_p(void* s){
	disable_irq(CLOCK_IRQ);
	SEMAPHORE* sem=(SEMAPHORE*) s;
	//if(p_proc_ready->is_reader==0){
	//	is_writing_now=1;
	//}
	sem->sem_num=sem->sem_num-1;

	if(sem->sem_num<0){
		//this means no enough resources, and the abs(num) indicates waiting num
		int chosen_restore_index=-(sem->sem_num)-1;
		p_proc_ready->rejected=1;
		//add the rejected process into the waiting queue
		sem->queue[chosen_restore_index]=p_proc_ready;
		
		enable_irq(CLOCK_IRQ);
		schedule();
	}
	else{
		enable_irq(CLOCK_IRQ);
	}
}

PUBLIC void sys_v(void* s){
	disable_irq(CLOCK_IRQ);
	SEMAPHORE* sem=(SEMAPHORE*) s;
	//if(p_proc_ready->is_reader==0){
	//	is_writing_now=0;
	//}
	sem->sem_num=sem->sem_num+1;

	if(sem->sem_num<=0){
		//we need to reset the rejected val of the first waiting pro in queue
		if(is_reader_first==1){
			int q_size=-(sem->sem_num)+1;
			for(int i=0;i<q_size;i++){
				if(sem->queue[i]->is_reader==1){
					//if it's reader first, consider recovering reader process
					sem->queue[i]->rejected=0;
					for(int j=i;j<q_size-1;j++){
						//eliminate it from the waiting queue
						sem->queue[j]=sem->queue[j+1];
					}
				}
			}
			//then consider writer queue, since it's reader first
			for(int i=0;i<q_size;i++){
				if(sem->queue[i]->is_reader==0){
					sem->queue[i]->rejected=0;
					for(int j=i;j<q_size-1;j++){
						sem->queue[j]=sem->queue[j+1];
					}
				}
			}
		}
		else{
			int q_size=-(sem->sem_num)+1;
			for(int i=0;i<q_size;i++){
				int has_dequeue=0;
				if(sem->queue[i]->is_reader==0){
					sem->queue[i]->rejected=0;
					has_dequeue=1;
					for(int j=i;j<q_size-1;j++){
						sem->queue[j]=sem->queue[j+1];
					}
					if(has_dequeue==1){
						break;
					}
				}
			}

			for(int i=0;i<q_size;i++){
				if(sem->queue[i]->is_reader==1){
					sem->queue[i]->rejected=0;
					for(int j=i;j<q_size-1;j++){
						sem->queue[j]=sem->queue[j+1];
					}
				}
			}
		}
	}
	enable_irq(CLOCK_IRQ);
}
