
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);

// my definition
//value from outside part

/**used to solve backspcae for tab */
extern unsigned int tabs[100];
extern int tabs_len;

/**used to solve backspace for \n */
extern int spcs_len;
extern unsigned int spcs[100];
extern int e_tabs_len;
extern unsigned int e_tabs[100]; //stands for enter_tabs

//useful flag for esc function
extern int esc_mode;
extern int color; //1 means in find mode color, 0 means common
extern u32 find_content[100];
extern int content_len;

//function part
//transfer empt and find from tty to console
PUBLIC void console_esc_empt(CONSOLE * p_con);
PUBLIC void console_esc_find(CONSOLE * p_con);


/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	set_cursor(p_tty->p_console->cursor);
}

/*====================================================*/
//try to fix bug
PUBLIC int equals_u32_u8(u8* p_vmem, u32 ch){
	if((int)(*(p_vmem)) >= 65 && (int)(*(p_vmem)) <= 90){
		//capital char
		if((int)(ch >> 28) != 0x2a){
			//judge if there is left shift
			return 0;
		}
		else{
			if((int)(*(p_vmem))==65 && (ch^0x2a1e9eaa)==0){
				//A
				return 1;
			}
			else if((int)(*(p_vmem))==66 && (ch^0x2a30b0aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==90 && (ch^0x2a2cacaa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==67 && (ch^0x2a2eaeaa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==68 && (ch^0x2a20a0aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==69 && (ch^0x2a1292aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==70 && (ch^0x2a21a1aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==71 && (ch^0x2a22a2aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==72 && (ch^0x2a23a3aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==73 && (ch^0x2a1797aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==74 && (ch^0x2a24a4aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==75 && (ch^0x2a25a5aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==76 && (ch^0x2a26a6aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==77 && (ch^0x2a32b2aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==78 && (ch^0x2a31b1aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==79 && (ch^0x2a1898aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==80 && (ch^0x2a1999aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==81 && (ch^0x2a1019aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==82 && (ch^0x2a1393aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==83 && (ch^0x2a1f9faa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==84 && (ch^0x2a1494aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==85 && (ch^0x2a1696aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==86 && (ch^0x2a2fafaa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==87 && (ch^0x2a1191aa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==88 && (ch^0x2a2dadaa)==0){
				return 1;
			}
			else if((int)(*(p_vmem))==89 && (ch^0x2a1595aa)==0){
				return 1;
			}
			else{
				return 0;
			}
		}
	}
	else{
		if(*(p_vmem)==ch){
			return 1;
		}
		else{
			return 0;
		}
	}
}

/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   console_esc_empt
 *======================================================================*/
PUBLIC void console_esc_empt(CONSOLE * p_con){

    char output_color =  DEFAULT_CHAR_COLOR;
    int i=0;
    for(i=0;i<content_len;i++){
        p_con->cursor--;
		//this is the base start point for esc mode
        u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
		if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1) {                       
			*p_vmem++ = ' ';
			*p_vmem++ = output_color;
		}
		set_cursor(p_con->cursor);
    }

	//this is a temp ptr, used to find the start point before esc input
    int ptr=0;
    output_color = DEFAULT_CHAR_COLOR;;
    u8* p_vmem; //this is the base
    
	for(ptr=p_con->original_addr;ptr<V_MEM_BASE + p_con->cursor * 2-content_len;ptr+=2){
		p_vmem = (u8*)(V_MEM_BASE + ptr);
		int i=0;
        if(*(p_vmem)==find_content[0]){
        	for(i=1;i<content_len;i++){
               p_vmem=(u8*)(V_MEM_BASE + ptr+i*2);
               if(*(p_vmem)!=find_content[i])
               	   break;
        	}
        	if(i==content_len){
               for(i=0;i<content_len;i++){              	
                	p_vmem = (u8*)(V_MEM_BASE + ptr+i*2);
			        *p_vmem++;
			        *p_vmem++ = output_color;                    
                }
        	}
        }

	}
}

/*======================================================================*
			   console_esc_find
 *======================================================================*/
PUBLIC void console_esc_find(CONSOLE * p_con){
	int ptr=0;
    char output_color =0x02; //color set to another one
    u8* p_vmem;
    
	for(ptr=p_con->original_addr;ptr<V_MEM_BASE + p_con->cursor * 2-content_len;ptr+=2){
		p_vmem = (u8*)(V_MEM_BASE + ptr);
		int len_match=0;
		//using to track the matching length
        if(*(p_vmem)==find_content[0]){ //the first letter is matched
        	for(len_match=1;len_match<content_len;len_match++){
				//match the following letters
                p_vmem=(u8*)(V_MEM_BASE + ptr+len_match*2);
                if(*(p_vmem)!=find_content[len_match]){
					//not mathced, start next round
					//important: should start right here, because
					//the current letter may match the input head
            		break;
				}
        	}
        	if(len_match==content_len){// successfully match to the end
               for(len_match=0;len_match<content_len;len_match++){
				   //start from the first letter, 'add' color
                    p_vmem = (u8*)(V_MEM_BASE + ptr+len_match*2);
			        *p_vmem++ ;
			        *p_vmem++ = output_color;
                }
        	}
        }
	}
}

/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
        disp_pos = 0;
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
        char output_color =  DEFAULT_CHAR_COLOR;
	switch(ch) {
	case '\n':
		if (p_con->cursor < p_con->original_addr +
		    p_con->v_mem_limit - SCREEN_WIDTH) {
			p_con->cursor = p_con->original_addr + SCREEN_WIDTH * 
				((p_con->cursor - p_con->original_addr) /
				 SCREEN_WIDTH + 1);
		}
		break;
	case '\b':
		if (p_con->cursor > p_con->original_addr) {
            if (tabs_len > 0 && p_con->cursor == tabs[tabs_len-1]+4){
                tabs_len--;
                p_con->cursor -= 4;
            }
            else {
				//backspace anyway
				p_con->cursor--;
				*(p_vmem-2) = ' ';
				*(p_vmem-1) = DEFAULT_CHAR_COLOR;
                p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
				//if last one is \n, current cursor will be at the rightest
                if ((p_con->cursor - p_con->original_addr+1) % SCREEN_WIDTH  == 0)
                    while (*(p_vmem-2) == ' ' && p_con->cursor > 0){
						int encounter_spcs = 0;
						if(esc_mode==0){
							//not in the esc mode, ok
							if ((spcs_len > 0 && p_con->cursor == spcs[spcs_len-1]+1)
							 ||(e_tabs_len > 0 && p_con->cursor == e_tabs[e_tabs_len-1]+4)){
								encounter_spcs = 1;
								if(p_con->cursor == spcs[spcs_len-1]+1)
									spcs_len--;
								else e_tabs_len--;
							 }
						}
						if(encounter_spcs == 0){
							p_con->cursor--;
							p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
						}
                        if ((p_con->cursor - p_con->original_addr) % SCREEN_WIDTH  == 0||encounter_spcs==1) break;
                    }
                 }
		   }

		break;
	default:
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
                        switch(color)
                        {
                        case 0: 
                          output_color = DEFAULT_CHAR_COLOR;
                          break;
                        case 1:
                          output_color = 0x02;
                          break;
                       
                        }
						
			*p_vmem++ = ch;
			*p_vmem++ = output_color;
			p_con->cursor++;
		}
		break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}


/*======================================================================*
			   clean_console
 *======================================================================*/
PUBLIC void clear_screen(CONSOLE * p_con)
{
 while (p_con -> cursor != 1)
 {
   out_char(p_con,'\b');
 }
}