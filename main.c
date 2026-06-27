#define F_CPU 14745600UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#define T_START 184     // 타이머 시작값 (5ms 주기)
#define MAX_BEATS 20    // 최대 저장 가능한 박자 수
#define TOLERANCE 150   // 오차 허용 범위 (±150ms)

#define LED_OFF    0xFF
#define LED_RED    0xFC
#define LED_ORANGE 0xE3
#define LED_GREEN  0x1F
#define BUZZER_PIN (1 << PG3)

volatile unsigned long secret_pattern[MAX_BEATS]; // 저장된 비밀 리듬
volatile unsigned long input_pattern[MAX_BEATS];  // 현재 입력 중인 리듬
volatile unsigned long timer_tick = 0;            // 5ms 단위 카운터

volatile int beat_count = 0;       // 현재 입력된 박자 개수
volatile int saved_beat_count = 0; // 저장된 비밀번호의 총 박자 개수
volatile int input_complete = 0;   // 입력 완료 플래그 (SW3 누르면 1)
volatile unsigned long first_press_tick = 0; // 첫 박자 시간 (기준점)

volatile int is_unlocked = 0;      // 잠금 해제 상태 플래그
volatile int mode_record_req = 0;  // 설정 모드 진입 요청 플래그

// === LCD 제어 함수 ===
void lcd_write(char c){ PORTA=c; PORTG|=0x04; _delay_us(1); PORTG&=0xFB; _delay_us(250); }
void lcd_clear(void){ PORTG&=0xFE; _delay_us(1); lcd_write(0x01); _delay_ms(4); }
void lcd_init(void){ PORTG&=0xFE; _delay_ms(200); lcd_write(0x38); lcd_write(0x0F); lcd_write(0x01); _delay_ms(100); }
void lcd_gotoxy(unsigned char x, unsigned char y){ PORTG&=0xFE; _delay_us(1); if(y==0) lcd_write(0x80+x); else lcd_write(0xC0+x); _delay_ms(1); }
void lcd_puts(char *s){ PORTG|=0x01; _delay_us(1); while(*s) lcd_write(*s++); }
void cursor_off(void){ PORTG&=0xFE; _delay_ms(200); lcd_write(0x0C); _delay_ms(100); }

//비프음 함수
void beep(int ms) {
	PORTG |= BUZZER_PIN;
	for(int i=0; i<ms; i++) _delay_ms(1);
	PORTG &= ~BUZZER_PIN;
}

// === ISR ===
ISR(TIMER0_OVF_vect) {
	TCNT0 = T_START;
	timer_tick++;
}

ISR(INT4_vect) {
	static unsigned long last_press_tick = 0;
	unsigned long current_tick = timer_tick;
	
	// 디바운싱 (50ms)
	if (current_tick - last_press_tick > 10) {
		if (beat_count < MAX_BEATS) {
			beep(30); // 클릭음
			
			// 첫 박자는 0, 이후 박자는 첫 박자와의 간격 저장
			if (beat_count == 0) {
				first_press_tick = current_tick;
				input_pattern[0] = 0;
				} else {
				input_pattern[beat_count] = (current_tick - first_press_tick) * 5;
			}
			beat_count++;
		}
		last_press_tick = current_tick;
	}
}

ISR(INT5_vect) {
	_delay_ms(20);
	beep(50);
	input_complete = 1;
}

ISR(INT6_vect) {
	if (is_unlocked == 1) {
		_delay_ms(20);
		beep(30);
		mode_record_req = 1;
	}
}

void init_devices() {
	DDRA = 0xFF; PORTA = 0xFF;
	DDRG = 0xFF; PORTG = 0x00;
	DDRB = 0xFF; PORTB = LED_OFF;
	
	lcd_init();
	cursor_off();

	EICRB = 0x2A;
	EIMSK = 0x70;
	
	TCCR0 = 0x07; TIMSK = 0x01; TCNT0 = T_START;
	SREG = 0x80;
}

// === 설정 모드 함수 ===
void run_setup_mode() {
	char buffer[16];
	mode_record_req = 0;
	
	lcd_clear(); lcd_puts("SETUP MODE");
	lcd_gotoxy(0, 1); lcd_puts("Tap SW2 -> SW3");
	PORTB = LED_ORANGE; beep(500);
	
	beat_count = 0; input_complete = 0;
	
	// 입력 대기 루프
	while (!input_complete) {
		if (beat_count > 0) {
			lcd_gotoxy(12, 0); sprintf(buffer, "%d   ", beat_count); lcd_puts(buffer);
		}
		// 5초간 입력없으면 리셋
		if (beat_count > 0 && (timer_tick - (input_pattern[beat_count-1]/5 + first_press_tick) > 1000)) {
			beat_count = 0; beep(200);
			lcd_gotoxy(11, 0); lcd_puts("Retry"); _delay_ms(1000); lcd_gotoxy(11, 0); lcd_puts("     ");
		}
	}
	
	// 입력된 패턴 저장
	saved_beat_count = beat_count;
	for(int k=0; k<saved_beat_count; k++) secret_pattern[k] = input_pattern[k];
	
	lcd_clear(); lcd_puts("New Password");
	lcd_gotoxy(0, 1); sprintf(buffer, "Saved %d Beats!", saved_beat_count); lcd_puts(buffer);
	
	PORTB = LED_GREEN;
	beep(50); _delay_ms(50); beep(50); _delay_ms(50); beep(50); _delay_ms(2000);
}

// === 리듬 검증 함수 ===
int verify_rhythm() {
	if (beat_count != saved_beat_count) return 0;
	for (int i = 1; i < saved_beat_count; i++) {
		long diff = (long)input_pattern[i] - (long)secret_pattern[i];
		if (abs(diff) > TOLERANCE) return 0;
	}
	return 1;
}

int main(void) {
	char buffer[16];
	init_devices();

	// 초기 부팅 화면
	lcd_clear(); lcd_puts("Welcome!");
	lcd_gotoxy(0, 1); lcd_puts("Init System...");
	beep(100); _delay_ms(1500);
	
	// 첫 실행 시 비밀번호 설정
	run_setup_mode();

	while (1) {
		// 잠금 대기 상태
		is_unlocked = 0;
		mode_record_req = 0;
		beat_count = 0;
		input_complete = 0;
		
		PORTB = LED_OFF;
		lcd_clear(); lcd_gotoxy(0, 0); lcd_puts("Rhythm Lock");
		lcd_gotoxy(0, 1); lcd_puts("Locked [SW2]");

		//입력 대기 (SW3 누를 때까지)
		while (!input_complete) {
			if (beat_count > 0) {
				lcd_gotoxy(13, 1); sprintf(buffer, "%d   ", beat_count); lcd_puts(buffer);
			}
			// 타임아웃 (3초간 입력 없으면 초기화)
			if (beat_count > 0 && (timer_tick - (input_pattern[beat_count-1]/5 + first_press_tick) > 600)) {
				beat_count = 0; beep(100);
				lcd_gotoxy(13, 1); lcd_puts("RST"); _delay_ms(500); lcd_gotoxy(13, 1); lcd_puts("   ");
			}
		}

		// 결과 분석
		lcd_clear(); lcd_puts("Checking...");
		PORTB = LED_ORANGE;
		_delay_ms(500);

		if (verify_rhythm()) {
			is_unlocked = 1;
			beep(100); _delay_ms(100); beep(100);
			
			lcd_clear(); lcd_puts("DOOR OPENED!");
			lcd_gotoxy(0, 1); lcd_puts("[SW4=Set]");
			PORTB = LED_GREEN;
			
			// 5초간 대기하며 SW4(패턴 초기화) 입력 감시
			for (int i = 0; i < 50; i++) {
				_delay_ms(100);
				if (mode_record_req == 1) break;
			}
			
			if (mode_record_req == 1) run_setup_mode();

			} else {
			// === 실패 ===
			lcd_clear(); lcd_puts("Access Denied");
			PORTB = LED_RED;
			beep(200); _delay_ms(100); beep(200);
			_delay_ms(1500);
		}
	}
}