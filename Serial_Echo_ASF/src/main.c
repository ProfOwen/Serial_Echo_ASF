/************************************************************************/
/* Tx to SAMD20 and have it echo back via serial using ASF              */
/* Designed by: Justin Owen												*/
/************************************************************************/

#include <asf.h>

/************************************************************************/
/* Global Variables                                                     */
/************************************************************************/

struct usart_module usart_instance;
#define MAX_RX_BUFFER_LENGTH   5
volatile uint8_t rx_buffer[MAX_RX_BUFFER_LENGTH];
volatile unsigned int sys_timer1 = 0;

/************************************************************************/
/* Function Prototypes                                                  */
/************************************************************************/

void usart_read_callback(const struct usart_module *const usart_module);
void usart_write_callback(const struct usart_module *const usart_module);

/************************************************************************/
/* INIT Clocks                                                          */
/************************************************************************/

void enable_tc_clocks(void)
{
	
	struct system_gclk_chan_config gclk_chan_conf;
	
	/* Turn on TC module in PM */
	system_apb_clock_set_mask(SYSTEM_CLOCK_APB_APBC, PM_APBCMASK_TC3);

	/* Set up the GCLK for the module */
	system_gclk_chan_get_config_defaults(&gclk_chan_conf);
	
	//Setup generic clock 0 (also the clock for MCU (running at 8 Mhz) as source for the timer clock)
	gclk_chan_conf.source_generator = GCLK_GENERATOR_0;
	system_gclk_chan_set_config(TC3_GCLK_ID, &gclk_chan_conf);
	
	//Enable the generic clock for the Timer/ Counter block
	system_gclk_chan_enable(TC3_GCLK_ID);
}

/************************************************************************/
/* INIT USART                                                           */
/************************************************************************/

void configure_usart(void)
{
	struct usart_config config_usart;
	usart_get_config_defaults(&config_usart);
	config_usart.baudrate    = 115200;
	config_usart.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
	config_usart.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
	config_usart.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
	config_usart.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
	config_usart.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;
	while (usart_init(&usart_instance,
	EDBG_CDC_MODULE, &config_usart) != STATUS_OK) {
	}
	usart_enable(&usart_instance);
}

void configure_usart_callbacks(void)
{
	usart_register_callback(&usart_instance,
	usart_write_callback, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_register_callback(&usart_instance,
	usart_read_callback, USART_CALLBACK_BUFFER_RECEIVED);
	usart_enable_callback(&usart_instance, USART_CALLBACK_BUFFER_TRANSMITTED);
	usart_enable_callback(&usart_instance, USART_CALLBACK_BUFFER_RECEIVED);
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

int main (void)
{
	system_init();
	system_clock_init();
	
	enable_tc_clocks();
	configure_usart();
	configure_usart_callbacks();
	
	system_interrupt_enable_global();
	
	 uint8_t string[] = "Hello World!\r\n";
	 usart_write_buffer_job(&usart_instance, string, sizeof(string));

	while (1) 
	{
		 usart_read_buffer_job(&usart_instance, (uint8_t *)rx_buffer, MAX_RX_BUFFER_LENGTH);
	}
}

/************************************************************************/
/* Subroutines                                                          */
/************************************************************************/

void usart_read_callback(const struct usart_module *const usart_module)
{
	usart_write_buffer_job(&usart_instance,
	(uint8_t *)rx_buffer, MAX_RX_BUFFER_LENGTH);
}

void usart_write_callback(const struct usart_module *const usart_module)
{
	port_pin_toggle_output_level(LED_0_PIN);
}


/************************************************************************/
/* Not used in the implementation. If you need a software timer         */
/* remember to call SysTick_Config(uint32_t)							*/
/************************************************************************/ 
 						
void SysTick_Handler(void)
{
	sys_timer1++;
}
