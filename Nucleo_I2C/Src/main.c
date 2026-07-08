#include <stdint.h>

// ─── Base Addresses ───────────────────────────────────────────
#define RCC_BASE     0x40021000
#define GPIOA_BASE   0x40010800
#define GPIOB_BASE   0x40010C00
#define I2C2_BASE    0x40005800
#define GPIOC_BASE   0x40011000
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

// ─── GPIOB Registers ──────────────────────────────────────────
#define GPIOB_CRH    (*(volatile uint32_t *)(GPIOB_BASE + 0x04))

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

// ─── I2C2 Registers ───────────────────────────────────────────
#define I2C2_CR1     (*(volatile uint32_t *)(I2C2_BASE + 0x00))
#define I2C2_CR2     (*(volatile uint32_t *)(I2C2_BASE + 0x04))
#define I2C2_DR      (*(volatile uint32_t *)(I2C2_BASE + 0x10))
#define I2C2_SR1     (*(volatile uint32_t *)(I2C2_BASE + 0x14))
#define I2C2_SR2     (*(volatile uint32_t *)(I2C2_BASE + 0x18))
#define I2C2_CCR     (*(volatile uint32_t *)(I2C2_BASE + 0x1C))
#define I2C2_TRISE   (*(volatile uint32_t *)(I2C2_BASE + 0x20))

// ─── MPU6050 ──────────────────────────────────────────────────
#define MPU6050_ADDR        0x68
#define MPU6050_PWR_MGMT_1  0x6B
#define MPU6050_ACCEL_XOUT_H 0x3B

// ─── Pin Definitions ──────────────────────────────────────────
#define BTN_PIN      13

// ─── State Machine ────────────────────────────────────────────
typedef enum {
    STATE_I2C_OFF,
    STATE_I2C_ON
} State;

// ─── Global Variables ─────────────────────────────────────────
volatile uint32_t tick_count = 0;
volatile uint8_t  btn_event  = 0;

// ─── SystemInit ───────────────────────────────────────────────
void SystemInit(void) {
}

// ─── SysTick Handler ──────────────────────────────────────────
void SysTick_Handler(void) {
    tick_count++;
}

// ─── Button Interrupt ─────────────────────────────────────────
void EXTI15_10_IRQHandler(void) {
    if(EXTI_PR & (1 << BTN_PIN)) {
        btn_event = 1;
        EXTI_PR = (1 << BTN_PIN);
    }
}

// ─── get_tick ─────────────────────────────────────────────────
uint32_t get_tick(void) {
    return tick_count;
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
    USART2_BRR   =   0x139;        // 115200 baud
    USART2_CR1  |=  (1 << 3);      // TE
    USART2_CR1  |=  (1 << 2);      // RE
    USART2_CR1  |=  (1 << 5);      // RXNEIE
    NVIC_ISER1  |=  (1 << 6);      // IRQ38
}

// ─── GPIO Init ────────────────────────────────────────────────
void gpio_init(void) {
    RCC_APB2ENR |= (1 << 0);       // AFIOEN
    RCC_APB2ENR |= (1 << 3);       // IOPBEN
    RCC_APB2ENR |= (1 << 4);       // IOPCEN

    // PC13 → input pull-up (bits 23:20 of CRH)
    GPIOC_CRH   &= ~(0xF << 20);
    GPIOC_CRH   |=  (0x8 << 20);
    GPIOC_ODR   |=  (1 << BTN_PIN);
}

// ─── EXTI Init ────────────────────────────────────────────────
void exti_init(void) {
    AFIO_EXTICR4 &= ~(0xF << 4);
    AFIO_EXTICR4 |=  (0x2 << 4);   // PC13 → EXTI13

    EXTI_IMR  |= (1 << BTN_PIN);
    EXTI_FTSR |= (1 << BTN_PIN);

    NVIC_ISER1 |= (1 << 8);        // IRQ40
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

// ─── USART2 Send Signed Int16 ─────────────────────────────────
void usart2_send_int16(int16_t val) {
    if(val < 0) {
        usart2_transmit('-');
        usart2_send_number((uint32_t)(-val));
    } else {
        usart2_send_number((uint32_t)(val));
    }
}

// ─── I2C2 Init ────────────────────────────────────────────────
void i2c_init(void) {

    // 1. Enable I2C2 clock
    RCC_APB1ENR |= (1 << 22);      // I2C2EN

    // 2. Configure PB10 (SCL) → AF open-drain 2MHz
    //    bits 11:8 of CRH → CNF=11 MODE=10 → 0xE
    GPIOB_CRH   &= ~(0xF << 8);
    GPIOB_CRH   |=  (0xE << 8);

    // 3. Configure PB11 (SDA) → AF open-drain 2MHz
    //    bits 15:12 of CRH → 0xE
    GPIOB_CRH   &= ~(0xF << 12);
    GPIOB_CRH   |=  (0xE << 12);

    // 4. Set APB1 clock frequency in CR2
    //    Must be configured before PE = 1
    I2C2_CR2    &= ~(0x3F);        // clear FREQ bits 5:0
    I2C2_CR2    |=  (36 << 0);     // FREQ = 36MHz

    // 5. Set CCR for 100kHz standard mode
    //    CCR = 36MHz / (2 × 100kHz) = 180
    //    Must be configured before PE = 1
    I2C2_CCR     =  180;

    // 6. Set TRISE for standard mode
    //    TRISE = (1000ns / 27.7ns) + 1 = 37
    //    Must be configured before PE = 1
    I2C2_TRISE   =  37;

    // 7. Enable peripheral
    I2C2_CR1    |=  (1 << 0);      // PE = 1
}

// ─── I2C Send Address ─────────────────────────────────────────
// addr: 7-bit device address
// mode: 0 = write, 1 = read
void i2c_send_address(uint8_t addr, uint8_t mode) {
    uint32_t temp;

    // Generate START condition
    I2C2_CR1 |= (1 << 8);
    while(!(I2C2_SR1 & (1 << 0)));     // wait SB = 1

    // Send address + direction bit
    // mode=0 → LSB=0 → write
    // mode=1 → LSB=1 → read
    I2C2_DR = (addr << 1) | mode;
    while(!(I2C2_SR1 & (1 << 1)));     // wait ADDR = 1

    // Clear ADDR by reading SR1 then SR2
    temp = I2C2_SR1;
    temp = I2C2_SR2;
    (void)temp;
}

// ─── I2C Write Byte ───────────────────────────────────────────
void i2c_write(uint8_t byte) {
    while(!(I2C2_SR1 & (1 << 7)));     // wait TxE = 1
    I2C2_DR = byte;
}

// ─── I2C Stop ─────────────────────────────────────────────────
void i2c_stop(void) {
    while(!(I2C2_SR1 & (1 << 2)));     // wait BTF = 1
    I2C2_CR1 |= (1 << 9);              // generate STOP
}

// ─── I2C Repeated Start ───────────────────────────────────────
void i2c_repeated_start(void) {
    I2C2_CR1 |= (1 << 8);              // set START again
    while(!(I2C2_SR1 & (1 << 0)));     // wait SB = 1
}

// ─── I2C Read Byte (sends ACK after) ──────────────────────────
uint8_t i2c_read_byte(void) {
    while(!(I2C2_SR1 & (1 << 6)));     // wait RxNE = 1
    return (uint8_t)I2C2_DR;
}

// ─── MPU6050 Write Register ───────────────────────────────────
void mpu_write(uint8_t reg, uint8_t data) {
    i2c_send_address(MPU6050_ADDR, 0); // START + write address
    i2c_write(reg);                    // register address
    i2c_write(data);                   // data to write
    i2c_stop();                        // STOP
}

// ─── MPU6050 Read Accelerometer (6 bytes) ─────────────────────
void mpu6050_read_accel(int16_t *x, int16_t *y, int16_t *z) {

    uint8_t buf[6];

    // Phase 1 — point MPU6050 to ACCEL_XOUT_H register
    i2c_send_address(MPU6050_ADDR, 0);     // START + write
    i2c_write(MPU6050_ACCEL_XOUT_H);       // register 0x3B

    // Phase 2 — read 6 bytes
    i2c_repeated_start();
    i2c_send_address(MPU6050_ADDR, 1);     // repeated START + read

    // Enable ACK for bytes 0-4
    I2C2_CR1 |= (1 << 10);                // ACK = 1

    // Read first 3 bytes normally
    buf[0] = i2c_read_byte();             // ACCEL_XOUT_H
    buf[1] = i2c_read_byte();             // ACCEL_XOUT_L
    buf[2] = i2c_read_byte();             // ACCEL_YOUT_H

    // ── 3 bytes remain — special N>2 procedure ────────────────
    // Wait BTF: buf[3] in DR, buf[4] in shift register
    while(!(I2C2_SR1 & (1 << 2)));        // BTF = 1

    // Clear ACK — buf[5] will receive NACK
    I2C2_CR1 &= ~(1 << 10);               // ACK = 0

    // Read buf[3] — buf[4] moves DR, buf[5] starts receiving
    buf[3] = I2C2_DR;

    // Queue STOP — executes after buf[5] complete
    I2C2_CR1 |= (1 << 9);                 // STOP

    // Read buf[4] — already in DR
    buf[4] = I2C2_DR;

    // Wait for buf[5] then read
    while(!(I2C2_SR1 & (1 << 6)));        // RxNE = 1
    buf[5] = I2C2_DR;

    // Combine high and low bytes into signed 16-bit values
    *x = (int16_t)((buf[0] << 8) | buf[1]);
    *y = (int16_t)((buf[2] << 8) | buf[3]);
    *z = (int16_t)((buf[4] << 8) | buf[5]);
}

// ─── MPU6050 Init ─────────────────────────────────────────────
void mpu6050_init(void) {
    // Wake MPU6050 — write 0x01 to PWR_MGMT_1
    // Clears SLEEP bit, uses PLL with gyro X as clock
    mpu_write(MPU6050_PWR_MGMT_1, 0x01);

    // Wait 10ms for device to stabilize
    uint32_t start = get_tick();
    while(get_tick() - start < 10);
}

// ─── Main ─────────────────────────────────────────────────────
int main(void) {

    pll_init();
    systick_init();
    usart2_init();
    gpio_init();
    exti_init();
    i2c_init();
    mpu6050_init();

    usart2_send_string("MPU6050 Ready\r\n");
    usart2_send_string("Press button to start reading\r\n");

    State current_state = STATE_I2C_OFF;

    while(1) {

        // ── Button debounce ───────────────────────────────────
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

        // ── State Machine ─────────────────────────────────────
        switch(current_state) {

            case STATE_I2C_OFF:
                if(event) {
                    current_state = STATE_I2C_ON;
                    usart2_send_string("State: I2C ON\r\n");
                }
                break;

            case STATE_I2C_ON:
                if(event) {
                    current_state = STATE_I2C_OFF;
                    usart2_send_string("State: I2C OFF\r\n");
                } else {
                    int16_t x, y, z;
                    mpu6050_read_accel(&x, &y, &z);

                    usart2_send_string("X: ");
                    usart2_send_int16(x);
                    usart2_send_string(" Y: ");
                    usart2_send_int16(y);
                    usart2_send_string(" Z: ");
                    usart2_send_int16(z);
                    usart2_send_string("\r\n");

                    // 100ms between reads
                    uint32_t t = get_tick();
                    while(get_tick() - t < 100);
                }
                break;
        }
    }
}
