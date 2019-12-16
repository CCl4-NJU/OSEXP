
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                   
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)
PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

//my proto
//function part
/**accept input and call find func in console */
PRIVATE void tty_do_find(TTY* p_tty);

/**accept esc and call empty func in console */
PRIVATE void tty_do_empt(TTY* p_tty);

//global val related to esc function
int color= 0 ;
int esc_mode=0;
int esc_show=0;
u32 find_content[100];
int content_len=0;

//external variable part, flag to periodically clean the console
extern int clean_console;

//using to solve backspace for tab
unsigned int tabs[100];
int tabs_len;

//using these vars to solve backspace for \n
int is_tab = 0;
unsigned int spcs[100];
int spcs_len;
unsigned int e_tabs[100];
int e_tabs_len;

/*======================================================================*
                           task_tty
 *======================================================================*/
//add - add initialization and clean console op
PUBLIC void task_tty()
{
	TTY*	p_tty;
	init_keyboard();

	for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_tty(p_tty);
	}
	select_console(0);

	//initialize the flag for tab enter len
	tabs_len = 0;

	//initialize the flag for \n backspc
	e_tabs_len = 0;
	spcs_len = 0;

	//used to judge the storage of spcs[]
	is_tab = 0;

	//initialize the flag for esc mode
	esc_mode=0;
    esc_show=0;
    content_len=0;
    
	while (1) {
		for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
            if (clean_console == 1&& is_current_console(p_tty->p_console)){
                clean_console = 0;
				//require cleaning, a cycle ends here
                while(p_tty->p_console->cursor > 0){
		        	out_char(p_tty->p_console,'\b');
				}
			}
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
	}
}

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{   

	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key)
{
    char output[2] = {'\0', '\0'};
	int i =0;

	// 'bool' var to test current mode
	int is_esc_show_mode = 0;
	int is_esc_find_mode = 0;

	if((esc_mode==1&&esc_show==1)){
		is_esc_show_mode = 1;	//in esc func show mode, colored
	}
	if(esc_mode==1&&esc_show==0){
		is_esc_find_mode = 1;	//in esc func inpt mode, uncolored
	}

	unsigned int flag = 0;
    unsigned int k = 0 ;

    if (!(key & FLAG_EXT)&&!(is_esc_show_mode==1)) {
		//need to store input in the content[]
        if(is_esc_find_mode==1){
            find_content[content_len]=key;
            content_len++;
        }
		put_key(p_tty, key);
    }
    else {
        int raw_code = key & MASK_RAW;
        switch(raw_code) {
			case ESC:
				esc_mode=!esc_mode; //take another value
				if(esc_mode==0){
					//if already in esc mode, quit and clean esc input
					tty_do_empt(p_tty);
					break;
				}
				if(esc_mode==1){
					//if not in esc mode, change the input color
					color=1;
					//remember to disable clock, so no clean happens
					disable_irq(CLOCK_IRQ);  
				}   
			break;

            case ENTER:
				//different op for enter in diff modes
                if(esc_mode==1&&esc_show==1){
                    break;
				}
				if(esc_mode==1){
					esc_show=!esc_show;
				}
				if(esc_show==1&&esc_mode==1){
					tty_do_find(p_tty);
					break;
				}
			    put_key(p_tty, '\n');
			break;
			    
			case TAB:
			//tab is 4 space
                if(esc_mode==1&&esc_show==1){
                    break;
				}
				//spcs[] is used in put_key, need to help it judge
				is_tab = 1;
                for(i = 0;i < 4; ++i){
                    put_key(p_tty,' ');
				}
				is_tab = 0;

                tabs[tabs_len++]= p_tty->p_console->cursor;
				e_tabs[e_tabs_len++] = p_tty->p_console->cursor;
            break;

            case BACKSPACE:
                if(esc_mode==1&&esc_show==1){
					//not allow any input except esc
                    break;
				}
				put_key(p_tty, '\b');
			break;

            case UP:
                if(esc_mode==1&&esc_show==1){
                    break;
				}
                if ((key & FLAG_SHIFT_L == FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
				scroll_screen(p_tty->p_console, SCR_DN);
                }
			break;
			case DOWN:
				if(esc_mode==1&&esc_show==1){
					break;
				}
				if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
					scroll_screen(p_tty->p_console, SCR_UP);
				}
			break;
                
		case F1:
		case F2:
		case F3:
		case F4:
                   
		case F5:
                     
		case F6:
		case F7:
		case F8:
		case F9:
		case F10:
		case F11:
		case F12:
			/* Alt + F1~F12 */
			if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
				select_console(raw_code - F1);
			}
			break;
                default:
                        break;
                }
        }
}

/*======================================================================*
			      put_key
*======================================================================*/
//extern int is_tab;
PRIVATE void put_key(TTY* p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}

	if(esc_mode==0&&is_tab==0&&(int)key==32){
		//input is not tab but only space, record the pos
		spcs[spcs_len++]= p_tty->p_console->cursor;
	}
}

/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}


/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);
		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		out_char(p_tty->p_console, ch);
	}
}

/*======================================================================*
			      tty_do_find
				  ask the console to help finding
 *======================================================================*/
PRIVATE void tty_do_find(TTY* p_tty){
	//more convenient to do in console.c
	console_esc_find(p_tty->p_console);
}

/*======================================================================*
			      tty_do_empt
				  ask the console to help emptying
 *======================================================================*/
PRIVATE void tty_do_empt(TTY* p_tty){

	//need to clear the flag
	//since it's used in console.c to help empty
	esc_mode=esc_show=0;
	color=0;
	
    console_esc_empt(p_tty->p_console);
	//before the statement, content_len is useful for locate cursor
    content_len=0;

	//reenable the interrupt
    enable_irq(CLOCK_IRQ);     
}
