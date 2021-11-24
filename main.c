#include "stm8s.h"
#include "debug_lib.h"
//#include "stdio.h"
//#include "math.h"

volatile uint16_t count;
volatile char res[7];
volatile char int_res[6];
volatile uint8_t int_len;
volatile uint16_t data[256];
volatile uint8_t data1[256];
volatile uint8_t pntr;
volatile uint16_t offset;
volatile uint16_t adc_val;
volatile uint16_t adc_val_old;
volatile uint16_t adc_val_trusted;
volatile uint16_t adc_val_trusted_old;
volatile uint16_t voltage;
volatile uint16_t voltage_trusted;
volatile uint16_t max_voltage;
volatile uint16_t dif_voltage;
volatile uint8_t ch_state;
volatile uint8_t led_blink;
volatile uint8_t ch_is_on;
volatile uint8_t dV_dt;
volatile uint8_t dvdt_min;
volatile uint8_t dvdt_max;
volatile uint16_t time_base;
volatile uint16_t number_total;
//volatile uint8_t number_to_trust;
volatile uint16_t adc_avrg[6];
volatile uint8_t adc_avrg_p;
volatile char ext_cmd[5];
volatile uint8_t ext_cmd_pntr;
volatile uint16_t arg;
volatile uint16_t curr;
volatile uint8_t curr_mode;
volatile uint8_t curr_offset;
volatile uint16_t cc_reg;
volatile uint8_t curr_pause;
volatile uint8_t curr_state;
volatile uint8_t curr_cnt;

//static void delay(uint32_t t)
//{
//    while(t--);
//}

void vlt_to_str(uint16_t c) {
    uint8_t i;
    i = 6;
    while (i > 0) {
		if (i != 5) {
			res[i - 1] = c % 10 + 48;
			c = c / 10;
		}
        i--;
    }
}

void int_to_str(uint16_t c) {
    uint8_t i;
    i = int_len;
    while (i > 0) {
		int_res[i - 1] = c % 10 + 48;
		c = c / 10;
        i--;
    }
    int_res[int_len] = 0; //end of string
}

void send_int(uint16_t i) {
	int_to_str(i);
	send_str(int_res);
}

void ch_on1() {
	TIM2_SetCompare3(cc_reg + curr_offset);
}

void ch_on() {
    //GPIO_WriteLow(GPIOC, GPIO_PIN_3);
	int16_t diff = adc_val - adc_val_old;
	switch (curr_mode) {
		case 1:
			if (cc_reg > curr * 2) {
				cc_reg--;
			}
			if (cc_reg < curr * 2) {
				cc_reg++;
			}
			break;
		case 2:
		//case 3:
			if (curr_state == 0) {
				if (adc_val < 794) { //1200mV
					if (diff < 1) {
						cc_reg += 1 - diff;
						curr_cnt++;
						if (curr_cnt > 6) {
							curr_state = 2;
							cc_reg = 0;
							curr_cnt = 0;
						}
					} else {
						curr_cnt = 0;
					}
				} else {
					curr_state++;
					curr_pause = 0;
				}
			} else if (curr_state == 1) {
				if (curr_pause < 10) {
					curr_pause++;
				} else {
					cc_reg = 0;
					curr_state++;
					curr_pause = 0;
				}
			} else if (curr_state == 2) {
				if (adc_val >= 744) { //1200mV
					curr_pause++;
					if (curr_pause > 20) {
					curr_state++;
					}
				} else {
					if (diff >= 0) {
						curr_cnt++;
						if (curr_cnt > 6) {
							curr_state = 0;
							curr_cnt = 0;
						}
					} else {
						curr_cnt = 0;
					}
				}
			} else if (curr_state == 3) {
				if (adc_val < 868) { //1400mV
					if (diff < 3) {
						cc_reg += 3 - diff;
					}
				}
			}
			break;
		default:
			cc_reg = curr * 2;
			break;
	}
	ch_on1();
	adc_val_old = adc_val;
	adc_val_trusted_old = adc_val_trusted;
}

void ch_off() {
    //GPIO_WriteHigh(GPIOC, GPIO_PIN_3);
	TIM2_SetCompare3(0);
}

void init() {
	count = 0;
	res[4] = 46;	// . symbol
    res[6] = 0;		//end of string

    max_voltage = 0;
    dif_voltage = 0;
	
	ch_state = 0;
    led_blink = 0;
	ch_is_on = 0;
	
	dV_dt = 0;
	dvdt_max = 0;
	dvdt_min = 255;
	time_base = 100;
	number_total = 0;
	//number_to_trust = 0;
	
	voltage = 0;
	voltage_trusted = 0;
	
	adc_val = 0;
	adc_val_old = 0;
	adc_val_trusted = 0;
	adc_val_trusted_old = 0;
	
	pntr = 0;
	offset = 0;
	
	ext_cmd_pntr = 0;
	ext_cmd[4] = 0;		//end of string
	
	curr = 10;
	curr_mode = 0;
	curr_offset = 4;
	cc_reg = 0;
	curr_state = 0;
}

uint8_t get_dvdt() {
	uint8_t i;
	uint16_t t;
	i = pntr;
	t = data[pntr];
	while (t < time_base && i > 0) {
		i--;
		t = t + data[i];
		
	}
	if (adc_val_trusted < offset + data1[i]) {
		return(0);
	}
	return(adc_val_trusted - offset - data1[i]);
}

uint16_t round(uint16_t r, uint16_t m1, uint16_t i1) {
	uint8_t n;
	uint8_t m;
	uint16_t i;
	if (data1[pntr] + offset - r == 1) {
		return(++r);
	}
	n = 4;
	i = 0;
	do {
		m = 10 * m1 / i1;
		m1 = 10 * m1 % i1;
		i++;
	} while (m == n && i < 3);
	if (m > n) r++;
	return(r);
}

void init_avrg() {
	uint8_t i;
	i = 0;
	while (i < 6) {
		adc_avrg[i] = 0;
		i++;
	}
	adc_avrg_p = 0;
}

uint16_t get_avrg(uint16_t d) {
	uint16_t i;
	uint16_t t;
	uint16_t r;
	uint16_t m;
	adc_avrg[adc_avrg_p] = d;
	adc_avrg_p++;
	if (adc_avrg_p >= 6) adc_avrg_p = 0;
	i = 0;
	t = 0;
	while (i < 6 && adc_avrg[i] > 0) {
		t = t + adc_avrg[i];
		i++;
	}
	r = t / i;
	m = t % i;
	return(round(r,m,i));
}

void inc_data1() {
	uint8_t i;
	i = 0;
	while (i < pntr) {
		data1[i]++;
		i++;
	}
}

uint8_t check_symbol(char s) {
	if (s < 48 || s > 57) {
		return(0);
	}
	return(1);
}
uint16_t get_arg() {
	uint16_t r;
	uint8_t i;
	uint8_t m;
	i = 3;
	r = 0;
	m = 1;
	while (i > 1) {
		if (!check_symbol(ext_cmd[i])) return(1000);
		r += m * (ext_cmd[i] - 48);
		m *= 10;
		i--;
	}
	return(r);
}
void set_curr(uint16_t c) {
	int_len = 3;
	send_str(" Old curr: ");
	send_int(curr);
	curr = c;
	send_str(" New curr: ");
	send_int(curr);
}
INTERRUPT_HANDLER(IRQ_Handler_TIM4, 23)
{
    uint8_t m = count % 100;
    if (m == 0) {
		if (led_blink == 2) {
			GPIO_WriteReverse(GPIOB, GPIO_PIN_5);
		}
		//if (curr_mode == 3 && ch_is_on) {
		//	if (count / 100 & 1) {
		//		ch_off();
		//	} else {
		//		ch_on1();
		//	}
		//}
		if (ch_is_on) {
			ch_on1();
		}
		
    }
	if (count == 15) {
		//if (ch_is_on) {
			ch_off();
		//}
	}
	if (count == 0) {
		if (led_blink < 2) {
			if (led_blink) {
				GPIO_WriteLow(GPIOB, GPIO_PIN_5);
			} else {
				GPIO_WriteHigh(GPIOB, GPIO_PIN_5);
			}
		}
		count = 1200;
		ADC1_StartConversion();
	}
    if (curr_mode > 2) {
		if (m == curr_mode + 1) {
			ch_off();
		}
		if (m == curr_mode - 1) {
			//GPIO_WriteHigh(GPIOC, GPIO_PIN_3);
			GPIOC->ODR |= (uint8_t)GPIO_PIN_3;
		}
		if (m == 1) {
			//GPIO_WriteLow(GPIOC, GPIO_PIN_3);
			GPIOC->ODR &= (uint8_t)(~(GPIO_PIN_3));
		}
	}
	count--;

    TIM4_ClearITPendingBit(TIM4_IT_UPDATE);
}

uint16_t get_vlt(uint16_t adc_val) {
	return(adc_val * 16 + adc_val * 11328 / 100000);
}

INTERRUPT_HANDLER(IRQ_Handler_ADC1, 22)
{
    adc_val = ADC1_GetConversionValue();
	voltage = get_vlt(adc_val);
    if (ch_state == 0) {
		adc_val_trusted = adc_val;
		voltage_trusted = voltage;
        if (voltage < 1600 * 10) {
            ch_state = 2;
			ch_is_on = 1;
			cc_reg = 0;
			curr_cnt = 0;
            max_voltage = voltage_trusted;
			number_total = 1;
			pntr = 0;
			data[pntr] = 0;
			data1[pntr] = 0;
			dV_dt = 0;
			dvdt_max = 0;
			dvdt_min = 255;
			time_base = 100;
			init_avrg();
			led_blink = 2;
			curr_state = 0;
			adc_val_old = adc_val;
	        adc_val_trusted_old = adc_val_trusted;
        }
    }
    else if (ch_state > 1) {
		number_total++;
		/*if (adc_val != adc_val_trusted) {
			if (number_to_trust < 4) {
				number_to_trust++;
			} else {
				adc_val_trusted = adc_val;
				number_to_trust = 0;
			}
		} else if (adc_val == adc_val_trusted) {
			number_to_trust = 0;
		}*/
		adc_val_trusted = get_avrg(adc_val);
		voltage_trusted = get_vlt(adc_val_trusted);
		
		if (voltage_trusted > max_voltage) {
			max_voltage = voltage_trusted;
		}
		dif_voltage = max_voltage - voltage_trusted;
		
		if (voltage > 1600 * 10) {
			ch_state = 1;
		}
		//wait for V > 1200 mV
		if (ch_state == 2) {
			if (voltage_trusted > 1200 * 10) {
				ch_state = 3;
				offset = adc_val_trusted;
				pntr = 255;
			}
		}
		if (ch_state > 2) {
			if (adc_val_trusted < adc_val_trusted_old && ch_state == 3) {
				offset = adc_val_trusted;
				pntr = 255;
				dvdt_max = 0;
			}
			if (adc_val_trusted != adc_val_trusted_old) {
				pntr++;
				while (adc_val_trusted < offset) {
					offset--;
					inc_data1();
				}
				data1[pntr] = adc_val_trusted - offset;
				data[pntr] = 0;
			}
			data[pntr]++;
		
			dV_dt = get_dvdt();
			
			//wait for dvdt decrease
			if (ch_state == 3) {
				if (dV_dt > dvdt_max) {
					dvdt_max = dV_dt;
				} else if (dvdt_max - dV_dt > 2 || dV_dt == 0 && dvdt_max > 0) {
					ch_state = 4;
					max_voltage = voltage_trusted;
					dif_voltage = 0;
				}
			}
			//wait for dvdt increase
			if (ch_state == 4) {
				if (dV_dt < dvdt_min) {
					dvdt_min = dV_dt;
				} else if (dV_dt - dvdt_min > 2 + 2 * (dvdt_min > dvdt_max >> 2)) {
					ch_state = 5;
					max_voltage = voltage_trusted;
					dif_voltage = 0;
					dvdt_max = 0;
				}
				if (time_base < data[pntr]) {
					time_base = data[pntr] ;
				}
			}
			//wait for dvdt decrease
			if (ch_state == 5) {
				if (dV_dt > dvdt_max) {
					dvdt_max = dV_dt;
				}
				if (dV_dt < dvdt_max) {
					if (dV_dt < (dvdt_max >> 3) + 1) {
						ch_state = 1;
					}
				}
				if (dif_voltage > 9 * 10) {
					ch_state = 1;
				}
			}
		}
	}
	else if (ch_state == 1) {
		if (ch_is_on) {
			ch_off();
			ch_is_on = 0;
			led_blink = 1;
		}
		adc_val_trusted = adc_val;
		voltage_trusted = voltage;
		if (voltage > 1600 * 10) {
			ch_state = 0;
			//max_voltage = 0;
			//dif_voltage = 0;
			led_blink = 0;
		}
	}

    
    /*if (voltage > 2000) {
        max_voltage = 0;
        dif_voltage = 0;
        GPIO_WriteHigh(GPIOB, GPIO_PIN_5);
    } else {
        GPIO_WriteLow(GPIOB, GPIO_PIN_5);
        if (voltage >= max_voltage) {
            max_voltage = voltage;
            dif_voltage = 0;
        } else {
            dif_voltage = max_voltage - voltage;
        }
        if (dif_voltage < 32) {
            delay(100000);
            GPIO_WriteHigh(GPIOB, GPIO_PIN_5);
        }
    }*/

	if (ext_cmd_pntr >= 4) {
		send_str("EXT_CMD: ");
		send_str(ext_cmd);
		//"c" - 99
		if (ext_cmd[0] == 99) {
			//"s" - 115
			if (ext_cmd[1] == 115) {
				arg = get_arg();
				if (arg < 1000) {
					set_curr(arg * 10);
				}
			}
			//"i" - 105
			if (ext_cmd[1] == 105) {
				arg = get_arg();
				if (arg < 1000) {
					set_curr(curr + arg);
				}
			}
			//"d" - 100
			if (ext_cmd[1] == 100) {
				arg = get_arg();
				if (arg < 1000) {
					set_curr(curr - arg);
				}
			}
			//"g" - 103
			if (ext_cmd[1] == 103) {
				set_curr(curr);
			}
			//"c" - 99
			//"z" - 122
			//"m" - 109
			if (ext_cmd[1] == 109) {
				arg = get_arg();
				if (arg < 1000) curr_mode = arg; 
			}
			//"o" - 111
			if (ext_cmd[1] == 111) {
				arg = get_arg();
				if (arg < 1000) curr_offset = arg;
			}
		}
		send_str("\n");
		ext_cmd_pntr = 0;
	}

	if (ch_is_on) {
		ch_on();
	}

	int_len = 4;
	send_str("A: ");
	send_int(adc_val);

	send_str(" AT: ");
	send_int(adc_val_trusted);

	send_str(" VT: ");
	vlt_to_str(voltage_trusted);
	send_str(res);

	send_str(" VM: ");
	vlt_to_str(max_voltage);
	send_str(res);

	send_str(" VD: ");
	vlt_to_str(dif_voltage);
	send_str(res);

	int_len = 5;
	send_str(" NT: ");
	send_int(number_total);

	send_str(" NC: ");
	send_int(data[pntr]);
	
	int_len = 3;
	send_str(" DVDT: ");
	send_int(dV_dt);

	send_str(" DMAX: ");
	send_int(dvdt_max);

	send_str(" DMIN: ");
	send_int(dvdt_min);

	send_str(" TB: ");
	send_int(time_base);

	send_str(" CC: ");
	send_int(cc_reg);

	send_str("\n");

    ADC1_ClearITPendingBit(ADC1_IT_EOC);
}

INTERRUPT_HANDLER(IRQ_Handler_UART1, 18) {
	if (ext_cmd_pntr < 4) {
		ext_cmd[ext_cmd_pntr] = UART1_ReceiveData8();
		ext_cmd_pntr++;
	}
	UART1_ClearITPendingBit(UART1_IT_RXNE);
}

int main( void ) {

    debug_init();
    //send_str("Starting...\n\r");
    //printf("Starting...\n\r");

    //PCB LED blinking
    GPIOB->CR1 &= (uint8_t)(~(GPIO_PIN_5)); //Open drain
    GPIOB->CR2 &= (uint8_t)(~(GPIO_PIN_5)); //No slope control
    GPIOB->DDR |= (uint8_t)GPIO_PIN_5;
    //GPIO_WriteLow(GPIOB, GPIO_PIN_5);
    //delay(100000);
    //GPIO_WriteHigh(GPIOB, GPIO_PIN_5);

	//Discharge driver
    GPIOC->CR1 |= (uint8_t)GPIO_PIN_3; //Push-pull
    GPIOC->CR2 &= (uint8_t)(~(GPIO_PIN_3)); //No slope control
    GPIOC->DDR |= (uint8_t)GPIO_PIN_3;
    //GPIO_Init(GPIOC,GPIO_PIN_3, GPIO_MODE_OUT_PP_HIGH_SLOW);
    //GPIO_WriteLow(GPIOC, GPIO_PIN_3);

    TIM4_DeInit();
    //TIM4_Cmd(DISABLE);
    TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);
    //TIM4_ClearFlag(TIM4_FLAG_UPDATE);
    TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);

	TIM2_DeInit();
	TIM2_TimeBaseInit(TIM2_PRESCALER_1, 999);
	// PWM1 Mode configuration: Channel3
	TIM2_OC3Init(TIM2_OCMODE_PWM1, TIM2_OUTPUTSTATE_ENABLE, 0, TIM2_OCPOLARITY_HIGH);
	TIM2_OC3PreloadConfig(ENABLE);
	TIM2_ARRPreloadConfig(ENABLE);
    
    //GPIO_Init(GPIOD,GPIO_PIN_3,GPIO_MODE_IN_FL_NO_IT);
    //GPIO_Init(GPIOD,GPIO_PIN_2,GPIO_MODE_IN_FL_NO_IT);
    
    
    ADC1_DeInit();
    
    /*-----------------CR1 & CSR configuration --------------------*/
    /* Configure the conversion mode and the channel to convert
        respectively according to ADC1_ConversionMode & ADC1_Channel values  &  ADC1_Align values */
    ADC1_ConversionConfig(ADC1_CONVERSIONMODE_SINGLE, ADC1_CHANNEL_4, ADC1_ALIGN_RIGHT);
    /* Select the prescaler division factor according to ADC1_PrescalerSelection values */
    ADC1_PrescalerConfig(ADC1_PRESSEL_FCPU_D18);

    /* Enable the ADC1 peripheral */
    ADC1->CR1 |= ADC1_CR1_ADON;
    
    /*ADC1_Init(ADC1_CONVERSIONMODE_SINGLE,
              ADC1_CHANNEL_4,
              ADC1_PRESSEL_FCPU_D8,
              ADC1_EXTTRIG_TIM,
              DISABLE,
              ADC1_ALIGN_RIGHT,
              ADC1_SCHMITTTRIG_CHANNEL2,
              DISABLE);*/
    ADC1_ITConfig(ADC1_IT_EOCIE ,ENABLE);

	init();

    enableInterrupts();
    
    TIM4_Cmd(ENABLE);
	TIM2_Cmd(ENABLE);
    


    while(1) {
        __asm__("WFI");
        //delay(100000);
        //GPIO_WriteLow(GPIOB, GPIO_PIN_5);
        //delay(100000);
        //GPIO_WriteHigh(GPIOB, GPIO_PIN_5);
        
    }
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
