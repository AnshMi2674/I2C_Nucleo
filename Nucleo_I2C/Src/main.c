#include <stdint.h>

// ─── Base Addresses ───────────────────────────────────────────
#define RCC_BASE     0x40021000
#define GPIOA_BASE   0x40010800		//USART
#define GPIOB_BASE   0x40010C00		//SCL, SDA
#define I2C2_BASE	 0x40005800
#define GPIOC_BASE   0x40011000		//BTN
#define AFIO_BASE    0x40010000
#define EXTI_BASE    0x40010400
#define FLASH_BASE   0x40022000
#define USART2_BASE  0x40004400

// ─── RCC Registers ────────────────────────────────────────────
#define RCC_APB1ENR  (*(volatile uint32_t *)(RCC_BASE + 0x1C))
#define RCC_APB2ENR  (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define RCC_CR       (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_CFGR     (*(volatile uint32_t *)(RCC_BASE + 0x04))

// ─── GPIOA Registers ──────────────────────────────────────────
#define GPIOA_CRL    (*(volatile uint32_t *)(GPIOA_BASE + 0x00))

// ─── GPIOB Registers ────────────────────────────────────────────────
#define GPIOB_CRH    (*(volatile uint32_t *)(GPIOB_BASE + 0x04))

// ─── I2C2 Registers ────────────────────────────────────────────────
#define I2C2_CR1	(*(volatile uint32_t *)(I2C2_BASE + 0x00))
#define I2C2_CR2	(*(volatile uint32_t *)(I2C2_BASE + 0x04))
#define I2C2_CCR	(*(volatile uint32_t *)(I2C2_BASE + 0x1C))
#define I2C2_TRISE	(*(volatile uint32_t *)(I2C2_BASE + 0x20))
#define I2C2_DR		(*(volatile uint32_t *)(I2C2_BASE + 0x10))
#define I2C2_SR1    (*(volatile uint32_t *)(I2C2_BASE + 0x14))
#define I2C2_SR2    (*(volatile uint32_t *)(I2C2_BASE + 0x18))

// ─── GPIOC Registers ──────────────────────────────────────────
#define GPIOC_CRH    (*(volatile uint32_t *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR    (*(volatile uint32_t *)(GPIOC_BASE + 0x0C))
#define GPIOC_IDR    (*(volatile uint32_t *)(GPIOC_BASE + 0x08))

// ─── AFIO Registers ───────────────────────────────────────────
#define AFIO_EXTICR4 (*(volatile uint32_t *)(AFIO_BASE + 0x14))

// ─── EXTI Registers ───────────────────────────────────────────
#define EXTI_IMR     (*(volatile uint32_t *)(EXTI_BASE + 0x00))
#define EXTI_FTSR    (*(volatile uint32_t *)(EXTI_BASE + 0x0C))
#define EXTI_PR      (*(volatile uint32_t *)(EXTI_BASE + 0x14))

// ─── Flash Register ───────────────────────────────────────────
#define FLASH_ACR    (*(volatile uint32_t *)(FLASH_BASE + 0x00))

// ─── SysTick Registers ────────────────────────────────────────
#define SYST_CSR     (*(volatile uint32_t *)0xE000E010)
#define SYST_RVR     (*(volatile uint32_t *)0xE000E014)
#define SYST_CVR     (*(volatile uint32_t *)0xE000E018)

// ─── NVIC ─────────────────────────────────────────────────────
#define NVIC_ISER1   (*(volatile uint32_t *)0xE000E104)

// ─── USART2 Registers ─────────────────────────────────────────
#define USART2_SR    (*(volatile uint32_t *)(USART2_BASE + 0x00))
#define USART2_DR    (*(volatile uint32_t *)(USART2_BASE + 0x04))
#define USART2_BRR   (*(volatile uint32_t *)(USART2_BASE + 0x08))
#define USART2_CR1   (*(volatile uint32_t *)(USART2_BASE + 0x0C))
#define USART2_CR2   (*(volatile uint32_t *)(USART2_BASE + 0x10))

#define BTN_PIN      13

// ─── Global Variables ─────────────────────────────────────────
volatile uint32_t tick_count = 0;
volatile uint8_t  rec_data   = 0;
volatile uint8_t  btn_event   = 0;
uint8_t rxe_data;
uint8_t adr = 0x68;

// ─── State Machine ────────────────────────────────────────────
typedef enum {
    STATE_I2C_OFF,
    STATE_I2C_ON
} State;

// ─── SystemInit ───────────────────────────────────────────────
void SystemInit(void) {
}

// ─── SysTick Handler ──────────────────────────────────────────
void SysTick_Handler(void) {
    tick_count++;
}

// ─── USART2 Interrupt ─────────────────────────────────────────
void USART2_IRQHandler(void) {
	if(USART2_SR & (1 << 5)) {     // RXNE set
	        rec_data = USART2_DR;       // reading DR clears RXNE
	    }
}

void EXTI15_10_IRQHandler(void) {
    if(EXTI_PR & (1 << BTN_PIN)) {
        btn_event = 1;
        EXTI_PR   = (1 << BTN_PIN);
    }
}

// ─── get_tick ─────────────────────────────────────────────────
uint32_t get_tick(void) {
    return tick_count;
}

// ─── String to Integer ────────────────────────────────────────
int str_to_int(const char *str) {
    int result = 0;
    while(*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

// ─── PLL Init ─────────────────────────────────────────────────
void pll_init(void) {
    RCC_CR      |=  (1 << 16);
    while(!(RCC_CR & (1 << 17)));

    FLASH_ACR   &= ~(0x7);
    FLASH_ACR   |=  (0x2);

    RCC_CFGR    &= ~(0xF << 4);
    RCC_CFGR    |=  (0x4 << 8);
    RCC_CFGR    &= ~(0x7 << 11);

    RCC_CFGR    |=  (1 << 16);
    RCC_CFGR    &= ~(1 << 17);
    RCC_CFGR    &= ~(0xF << 18);
    RCC_CFGR    |=  (0x7 << 18);

    RCC_CR      |=  (1 << 24);
    while(!(RCC_CR & (1 << 25)));

    RCC_CFGR    &= ~(0x3);
    RCC_CFGR    |=  (0x2);
    while((RCC_CFGR & (0x3 << 2)) != (0x2 << 2));
}

// ─── SysTick Init ─────────────────────────────────────────────
void systick_init(void) {
    SYST_RVR = 71999;
    SYST_CVR = 0;
    SYST_CSR = 0x7;
}

// ─── USART2 Init ──────────────────────────────────────────────
void usart2_init(void) {
    // Enable clocks
    RCC_APB1ENR |= (1 << 17);      // USART2EN
    RCC_APB2ENR |= (1 << 2);       // IOPAEN

    // PA2 → AF push-pull 10MHz (TX) bits 11:8
    GPIOA_CRL   &= ~(0xF << 8);
    GPIOA_CRL   |=  (0x9 << 8);

    // PA3 → floating input (RX) bits 15:12
    GPIOA_CRL   &= ~(0xF << 12);
    GPIOA_CRL   |=  (0x4 << 12);

    USART2_CR1  |=  (1 << 13);     // UE
    USART2_CR1  &= ~(1 << 12);     // M = 8 data bits
    USART2_CR2  &= ~(0x3 << 12);   // 1 stop bit
    USART2_BRR   =   0x139;        // 115200 baud at 36MHz PCLK1
    USART2_CR1  |=  (1 << 3);      // TE
    USART2_CR1  |=  (1 << 2);      // RE
    USART2_CR1  |=  (1 << 5);      // RXNEIE
    NVIC_ISER1  |=  (1 << 6);      // IRQ38 → USART2
}

// ─── GPIO Init ────────────────────────────────────────────────
void gpio_init(void) {
    RCC_APB2ENR |= (1 << 0);   // AFIOEN
    RCC_APB2ENR |= (1 << 4);   // IOPCEN

    RCC_APB2ENR |= (1 << 3);	//IOPBEN

    // PC13 → input pull-up (bits 23:20 of CRH)
    GPIOC_CRH   &= ~(0xF << 20);
    GPIOC_CRH   |=  (0x8 << 20);
    GPIOC_ODR   |=  (1 << BTN_PIN);
}

// ─── EXTI Init ────────────────────────────────────────────────
void exti_init(void) {
    AFIO_EXTICR4 &= ~(0xF << 4);
    AFIO_EXTICR4 |=  (0x2 << 4);

    EXTI_IMR  |= (1 << BTN_PIN);
    EXTI_FTSR |= (1 << BTN_PIN);

    NVIC_ISER1 |= (1 << 8);    // IRQ40 → bit 8 of ISER1
}

// ─── USART2 Transmit ──────────────────────────────────────────
void usart2_transmit(uint8_t data) {
    while(!(USART2_SR & (1 << 7)));
    USART2_DR = data;
}

// ─── USART2 Send String ───────────────────────────────────────
void usart2_send_string(const char *str) {
    while(*str) {
        usart2_transmit((uint8_t)*str++);
    }
}

// ─── USART2 Send Number ───────────────────────────────────────
void usart2_send_number(uint32_t num) {
    if(num == 0) { usart2_transmit('0'); return; }
    char buf[10];
    int i = 0;
    while(num > 0) { buf[i++] = '0' + (num % 10); num /= 10; }
    while(i > 0)   { usart2_transmit(buf[--i]); }
}

// ─── I2C Init ────────────────────────────────────────────────
void i2c_init(void) {

    // 1. Enable I2C2 clock (APB1)
    RCC_APB1ENR |= (1 << 22);      // I2C2EN

    // 2. Configure PB10 (SCL) and PB11 (SDA)
    //    AF open-drain, 2MHz
    //    PB10 → bits 11:8 of CRH  → 0xE(1110)
    //    PB11 → bits 15:12 of CRH → 0xE
    GPIOB_CRH   &= ~(0xF << 8);
    GPIOB_CRH   |=  (0xE << 8);    // PB10 SCL
    GPIOB_CRH   &= ~(0xF << 12);
    GPIOB_CRH   |=  (0xE << 12);   // PB11 SDA

    // 3. Set peripheral input clock frequency
    //    APB1 = 36MHz
    I2C2_CR2 &= ~(0x3F);     // clear bits 5:0
    I2C2_CR2 |=  (36 << 0);  // set FREQ = 36

    // 4. Configure CCR for 100kHz standard mode
    //    CCR = 36MHz / (2 × 100kHz) = 180
    I2C2_CCR     =  180;

    // 5. Configure TRISE
    //    Standard mode max rise = 1000ns
    //    TRISE = (1000ns × 36MHz) + 1 = 37
    I2C2_TRISE   =  37;

    // 6. Enable peripheral
    I2C2_CR1    |=  (1 << 0);      // PE = 1
}

// ─── I2C send address ────────────────────────────────────────────────
void i2c_send_address(uint8_t addr, uint8_t mode) {

	uint32_t temp;
	//Set the START bit in the I2C_CR1 register to generate a Start condition
	I2C2_CR1 |= (1 << 8);

	//Confirming SB bit in SR1
	while(!(I2C2_SR1 & (1 << 0)));		//Checking SB bit

	/*
	 *  7-bit addressing mode,
	 * – To enter Transmitter mode, a master sends the slave address with LSB reset.
	 * – To enter Receiver mode, a master sends the slave address with LSB set.
	*/

	//WRITE of slave address
	I2C2_DR = (addr << 1) | mode;

	//Confirming address transmission
	while(!(I2C2_SR1 & (1 << 1)));		//Checking ADDR bit

	// Clear ADDR
	temp = I2C2_SR1;
    temp = I2C2_SR2;
	(void)temp;
}

// ─── I2C write ────────────────────────────────────────────────
void i2c_write(uint8_t byte) {

	//Wait for TxE bit
	while(!(I2C2_SR1 & (1 << 7)));		//Polling TxE bit

	I2C2_DR = byte;
}

// ─── I2C Stop ────────────────────────────────────────────────
void i2c_stop(void) {

	//Checking whether any of TxE or BTF bit is set for programming STOP bit
	while(!(I2C2_SR1 & (1 << 7)) || !(I2C2_SR1 & (1 << 2)));

	I2C2_CR1 |= (1 << 9);
}

// ─── Repeat START bit ONLY ────────────────────────────────────────────────
void i2c_repeated_start(void) {
    I2C2_CR1 |= (1 << 8);              // set START bit again
    while(!(I2C2_SR1 & (1 << 0)));     // wait SB = 1
}

/*
// ─── I2C Stop Communication ────────────────────────────────────────────────
THis can be used to permanently stop i2c2 comm.
void i2c_stop_perm(void)
{
    // Wait until byte completely transmitted
    while(!(I2C2_SR1 & (1<<2)));      // BTF

    // Generate STOP
    I2C2_CR1 |= (1<<9);

    // Wait until bus becomes free
    while(I2C2_SR2 & (1<<1));         // BUSY

    // Disable peripheral
    I2C2_CR1 &= ~(1<<0);              // PE = 0
}

*/

// ─── I2C Receiver ────────────────────────────────────────────────
// Call this before starting to read bytes
void i2c_ack_enable(void) {
    I2C2_CR1 |= (1 << 10);   // ACK = 1
}

// Read one byte (ACK sent automatically because ACK bit = 1)
uint8_t i2c_read_byte(void) {
    while(!(I2C2_SR1 & (1 << 6)));   // wait RxNE
    return (uint8_t)I2C2_DR;
}

// Call before reading LAST byte
void i2c_nack(void) {
    I2C2_CR1 &= ~(1 << 10);  // ACK = 0 → NACK after next byte
}

// ─── MPU Write ────────────────────────────────────────────────
void mpu_write(uint8_t reg, uint8_t data) {

	i2c_send_address(adr, 0);	//sends address
	i2c_write(reg);				//points to register
	i2c_write(data);			//Writes the 'data' in pointed reg.
	i2c_stop();					//Stop communication
}

// ─── MPU Read ────────────────────────────────────────────────
void mpu6050_read_accel(int16_t *x, int16_t *y, int16_t *z) {

    uint8_t buf[6];

    // Phase 1 — write register address
    i2c_send_address(0x68, 0);      // START + write address
    i2c_write(0x3B);                // point to ACCEL_XOUT_H

    // Phase 2 — read 6 bytes
    i2c_repeated_start();
    i2c_send_address(0x68, 1);      // repeated START + read address

    // Enable ACK for first bytes
    I2C2_CR1 |= (1 << 10);          // ACK = 1

    // Read bytes 0, 1, 2 normally
    buf[0] = i2c_read_byte();       // ACCEL_XOUT_H
    buf[1] = i2c_read_byte();       // ACCEL_XOUT_L
    buf[2] = i2c_read_byte();       // ACCEL_YOUT_H

    // 3 bytes remain — special procedure
    // Step 1: wait BTF (buf[3] in DR, buf[4] in shift reg)
    while(!(I2C2_SR1 & (1 << 2)));  // BTF = 1

    // Step 2: clear ACK (buf[5] will get NACK)
    I2C2_CR1 &= ~(1 << 10);

    // Step 3: read buf[3] (buf[4] moves to DR, buf[5] starts)
    buf[3] = I2C2_DR;

    // Step 4: generate STOP immediately
    I2C2_CR1 |= (1 << 9);

    // Step 5: read buf[4] (already in DR)
    buf[4] = I2C2_DR;

    // Step 6: wait and read buf[5]
    while(!(I2C2_SR1 & (1 << 6)));  // wait RxNE
    buf[5] = I2C2_DR;

    // Combine bytes into 16-bit values
    *x = (int16_t)((buf[0] << 8) | buf[1]);
    *y = (int16_t)((buf[2] << 8) | buf[3]);
    *z = (int16_t)((buf[4] << 8) | buf[5]);
}

// ─── MPU Init ────────────────────────────────────────────────
void mpu6050_init(void) {
    // Write 0x01 to PWR_MGMT_1 (register 0x6B)
    // Clears SLEEP bit, uses PLL with gyro X as clock source
    mpu_write(0x6B, 0x01);

    // Wait 10ms for device to stabilize
    uint32_t start = get_tick();
    while(get_tick() - start < 10);
}

int main(void) {
    pll_init();
    systick_init();
    usart2_init();
    gpio_init();
    exti_init();
    i2c_init();
    mpu6050_init();
    State current_state = STATE_I2C_OFF;

    while(1) {

        // button event handling
    	uint8_t event = 0;
    	if(btn_event) {
    		uint32_t press_time = get_tick();
    	    while(!((GPIOC_IDR >> BTN_PIN) & 1)) {
    	        if(get_tick() - press_time > 20) {
    	            event = 1;
    	            break;
    	                }
    	            }
    	            btn_event = 0;
    	}

        switch(current_state) {
            case STATE_I2C_OFF:
                // do nothing
            	if(event) {
            	    current_state = STATE_I2C_ON;
            	    usart2_send_string("State: I2C ON\r\n");
            	}
                break;

            case STATE_I2C_ON:

            	if(event) {
            		current_state = STATE_I2C_OFF;
            		usart2_send_string("State: I2C OFF\r\n");
         	    }
            	else {


            		int16_t x, y, z;
            		mpu6050_read_accel(&x, &y, &z);
            		usart2_send_string("X: ");
            		usart2_send_number((uint32_t)(x < 0 ? -x : x));
            		usart2_send_string(" Y: ");
            		usart2_send_number((uint32_t)(y < 0 ? -y : y));
            		usart2_send_string(" Z: ");
            		usart2_send_number((uint32_t)(z < 0 ? -z : z));
            		usart2_send_string("\r\n");

            		// Small delay between readings
            		uint32_t t = get_tick();
            		while(get_tick() - t < 100);  // 100ms between reads
            		break;
            	}
        }
    }
}
