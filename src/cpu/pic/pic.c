#include "intrinsics.h"
#include "io_wait.h"
#include "pic.h"
/* ======================== Setting the extern constants ===================== */
// Not needed, static const will be optimized into a constant anyway

/* ========================================= */

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define ICW1_ICW4 0x01      /* Indicates that ICW4 will be present */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

#define CASCADE_IRQ 2

/*
arguments:
    offset1 - vector offset for master PIC
        vectors on the master become offset1..offset1+7
    offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(int offset1, int offset2)
{
    __outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); // starts the initialization sequence (in cascade mode)
    io_wait();
    __outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    __outb(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
    io_wait();
    __outb(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset
    io_wait();

    __outb(PIC1_DATA, 1 << CASCADE_IRQ); // ICW3: tell Master PIC that there is a slave PIC at IRQ2
    io_wait();
    __outb(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
    io_wait();

    __outb(PIC1_DATA, ICW4_8086); // ICW4: have the PICs use 8086 mode (and not 8080 mode)
    io_wait();
    __outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Unmask both PICs.
    __outb(PIC1_DATA, 0);
    __outb(PIC2_DATA, 0);
}

#define PIC_EOI 0x20 /* End-of-interrupt command code */

void PIC_sendEOI(uint8_t irq)
{
    if (irq >= 8)
	__outb(PIC2_COMMAND, PIC_EOI);

    __outb(PIC1_COMMAND, PIC_EOI);
}

// Set mask is to DISABLE THE IRQ
void IRQ_set_mask(uint8_t IRQline)
{
    uint16_t port;
    uint8_t  value;

    if (IRQline < 8)
    {
	port = PIC1_DATA;
    }
    else
    {
	port = PIC2_DATA;
	IRQline -= 8;
    }
    value = __inb(port) | (1 << IRQline);
    __outb(port, value);
}

// Clear mask is to ENABLE THE IRQ
void IRQ_clear_mask(uint8_t IRQline)
{
    uint16_t port;
    uint8_t  value;

    if (IRQline < 8)
    {
	port = PIC1_DATA;
    }
    else
    {
	port = PIC2_DATA;
	IRQline -= 8;
    }
    value = __inb(port) & ~(1 << IRQline);
    __outb(port, value);
}

void PIC_disable(void)
{
    __outb(PIC1_DATA, 0xff);
    __outb(PIC2_DATA, 0xff);
}

void initialize_irqs()
{

    for (int i = 0; i < 16; i++)
    {
	IRQ_set_mask(i);
    }
}
