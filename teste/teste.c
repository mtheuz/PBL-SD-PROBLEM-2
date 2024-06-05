#include <linux/module.h>
#include <stdint.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/fs.h>

#define WBR 0x00
#define WBM 0x01
#define WSM 0x02
#define DP  0x03
#define DATA_A  0x80
#define DATA_B  0x70
#define START 0xc0
#define LW_BRIDGE_BASE 0xFF200000 
#define LW_BRIDGE_SPAN 0x00005000 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matheus Mota, Pedro Henrique, Dermeval Neves");
MODULE_DESCRIPTION("Módulo de exemplo para envio de instruções");

volatile int *START_PTR;
volatile int *DATA_A_PTR;
volatile int *DATA_B_PTR;
void __iomem *LW_virtual;

static int device_open(struct inode *inodep, struct file *filep);
static int device_release(struct inode *inodep, struct file *filep);
static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset);

static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .write = device_write,
};

void send_instruction(volatile int opcode, volatile int dados) {
    *START_PTR = 0;
    *DATA_A_PTR = opcode;
    *DATA_B_PTR = dados;
    *START_PTR = 1;
    *START_PTR = 0;
}

void instrucao_wbr(int reg, int r, int g, int b, int x, int y, int sp) {
    volatile int opcode = WBR; // Opcode para WBR
    volatile int dados = (b << 16) | (g << 8) | r;
    volatile int opcode_reg = (opcode << 5) | (reg << 0);
    opcode_reg |= (x << 4) | y;
    if (sp) {
        opcode_reg |= (1 << 31);
    }
    send_instruction(opcode_reg, dados);
}

void mudar_bg_wbr (int reg, int r, int g, int b) {
    uint32_t dados = 0b0; 
    volatile int opcode = WBR; // Opcode para WBR
    volatile int dados = (b << 6) | (g << 3) | r;
    volatile int opcode_reg = (reg << 4) | opcode;
    send_instruction(opcode_reg, dados);
}

static int device_open(struct inode *inodep, struct file *filep) {
    return 0;
}

static int device_release(struct inode *inodep, struct file *filep) {
    return 0;
}

static ssize_t device_write(struct file *filp, const char *buffer, size_t len, loff_t *off) {
    unsigned char command[9]; 

    if (len < 5 || len > 9) {
        printk(KERN_ALERT "Comprimento de comando inválido\n");
        return -EINVAL;
    }

    if (copy_from_user(command, buffer, 9)) {
        return -EFAULT;
    }

    mudar_bg_wbr(command[1], command[2], command[3], command[4]);

    return len;
}

static int __init my_module_init(void) {
    LW_virtual = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    DATA_A_PTR = (volatile int *) (LW_virtual + DATA_A);
    DATA_B_PTR = (volatile int *) (LW_virtual + DATA_B);   
    START_PTR = (volatile int *) (LW_virtual + START);
    printk(KERN_INFO "Módulo carregado\n");
    return 0;
}

static void __exit my_module_exit(void) {
    *START_PTR = 0;
    iounmap(LW_virtual);
    printk(KERN_INFO "Módulo descarregado\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

}
