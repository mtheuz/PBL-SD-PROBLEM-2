#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/delay.h>

#define WBR 0b00
#define WBM 0b10
#define WSM 0b01
#define DP  0b11
#define DATA_A  0x80
#define DATA_B  0x70
#define START 0xc0
#define WRFULL 0xb0
#define LW_BRIDGE_BASE 0xFF200000 
#define LW_BRIDGE_SPAN 0x00005000 

#define DEVICE_NAME "gpu_driver"
#define CLASS_NAME "gpudriver_class"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matheus Mota, Pedro Henrique e Dermeval Neves");
MODULE_DESCRIPTION("Módulo de exemplo para envio de instruções");

// Declaração de variáveis globais
static int major_number;
static struct class* gpu_class = NULL;
static struct device* gpu_device = NULL;
static struct cdev gpu_cdev;

volatile int *START_PTR;
volatile int *WRFULL_PTR;
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
    /*START_PTR = 0;
    *DATA_A_PTR = opcode;
    *DATA_B_PTR = dados;
    *START_PTR = 1;*/

    iowrite32(0, START_PTR);
    iowrite32(opcode, DATA_A_PTR);
    iowrite32(dados, DATA_B_PTR);
    iowrite32(1, START_PTR);
    iowrite32(0, START_PTR);
}

//FUNÇÃO PSEUDO-FUNCIONAL
void instrucao_wbr(int b, int g, int r) {
    volatile int opcode = WBR; // Opcode para WBR
    volatile int dados = (b << 6) | (g << 3) | r;
    volatile int opcode_reg = (opcode);
    send_instruction(opcode_reg, dados);
}

void instrucao_wbr_sprite(int reg, int offset, int x, int y, int sp) {
    volatile int opcode = WBR; // Opcode para WBR
    volatile int opcode_reg = (reg << 4) | opcode ;
    volatile int dados = offset | (y << 9) | (x << 19); 
    if (sp) {
        dados |= (1 << 29);
    }
    send_instruction(opcode_reg, dados);
}

void instrucao_wbm(int address, int r, int g, int b) {
    volatile int opcode = WBM; // Opcode para WBM
    volatile int dados = (b << 6) | (g << 3) | r;
    volatile int opcode_reg = (address << 4) | opcode;
    send_instruction(opcode_reg, dados);
}

void instrucao_wsm(int address, int r, int g, int b) {
    volatile int opcode = WSM; // Opcode para WSM
    volatile int dados = (b << 6) | (g << 3) | r;
    volatile int opcode_reg = (address << 4) | opcode;
    send_instruction(opcode_reg, dados);
}

void instrucao_dp(int address, int ref_x, int ref_y, int size, int r, int g, int b, int shape) {
    volatile int opcode = DP; // Opcode para DP
    volatile int opcode_reg = (address << 4) | opcode;
    volatile int rgb = (b << 6) | (g << 3) | r;
    volatile int dados =  (rgb << 22) | (size << 18) | (ref_y << 9) | ref_x;
    if (shape) {
        dados |= (1 << 31);
    }
    send_instruction(opcode_reg, dados);
}

static int device_open(struct inode *inodep, struct file *filep) {
    return 0;
}

static int device_release(struct inode *inodep, struct file *filep) {
    return 0;
}

static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)  {
    unsigned char command[9]; 
   
    int buffer_gpu = ioread32(WRFULL_PTR);
    
    while (buffer_gpu){
        buffer_gpu = ioread32(WRFULL_PTR);
        if (buffer_gpu == 0){
            msleep(130);
        }
    };
    
    if (len < 4 || len > 9) {
        printk(KERN_ALERT "Comprimento de comando inválido\n");
        return -EINVAL;
    }

    if (copy_from_user(&command, buffer, sizeof(command))) {
        return -EFAULT;
    }

    switch (command[0]) {
        case 0:  {
            int r = command[1];
            int g = command[2];
            int b = command[3];
            instrucao_wbr(r, g, b);
            break;
        }
        case 1: {
            int reg = command[1];
            int offset = ((command[2] << 1) & 0x1FE) | ((command[3] >> 7) & 0x01); // 9-bit offset
            int x = ((command[3] << 3) & 0x3F8) | ((command[4] >> 5) & 0x07);     // 10-bit x
            int y = ((command[4] << 5) & 0x3E0) | ((command[5] >> 3) & 0x1F);     // 10-bit y
            int sp = command[6];
            instrucao_wbr_sprite(reg, offset, x, y, sp);
            break;
        }
        case 2: {
            int address = ((command[1] << 5) | (command[2] >> 3)); // 12-bit address
            int r = command[2] & 0b111;
            int g = command[3];
            int b = command[4];
            instrucao_wbm(address, r, g, b);
            break;
        }
        case 3: {
            int address = (command[1] << 6) | (command[2]); // 14-bit address
            int r = command[3];
            int g = command[4];
            int b = command[5];
            instrucao_wsm(address, r, g, b);
            break;
        }
        case 4: {
            int address = command[1];
            int ref_x = ((command[2] << 1) | command[3] >> 7);
            int ref_y = (((command[3] & 0b1111111) << 2) | command[4] >> 6);
            int size = command[4] & 0b1111;
            int r = command[5] >> 5;
            int g = (command[5] >> 2) & 0b111;
            int b =  command[6] >> 5;
            int shape =  command[6] & 0b1;
            instrucao_dp(address, ref_x, ref_y, size, r, g, b, shape);
            break;
        }
        default: {
            printk(KERN_ALERT "Comando desconhecido\n");
            return -EINVAL;
        }
    }

    return len;
}


static int __init my_module_init(void) {
    LW_virtual = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    printk(KERN_INFO "por favor\n");

    if (major_number < 0) {
        printk(KERN_ALERT "Falha ao registrar um número principal\n");
        return major_number;
    }

    gpu_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(gpu_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Falha ao registrar a classe do dispositivo\n");
        return PTR_ERR(gpu_class);
    }

    printk(KERN_INFO "Módulo carregado: classe do dispositivo criada corretamente\n");
    gpu_device = device_create(gpu_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(gpu_device)) {
        class_destroy(gpu_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Falha ao criar o dispositivo\n");
        return PTR_ERR(gpu_device);
    }

    LW_virtual = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    if (!LW_virtual) {
        device_destroy(gpu_class, MKDEV(major_number, 0));
        class_unregister(gpu_class);
        class_destroy(gpu_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Falha ao mapear a memória\n");
        return -ENOMEM;
    }


    DATA_A_PTR = (volatile int *) (LW_virtual + DATA_A);
    DATA_B_PTR = (volatile int *) (LW_virtual + DATA_B);   
    START_PTR = (volatile int *) (LW_virtual + START);
    WRFULL_PTR = (volatile int *) (LW_virtual + WRFULL);
    
    return 0;
}


static void __exit my_module_exit(void) {
    iounmap(LW_virtual);
    device_destroy(gpu_class, MKDEV(major_number, 0));
    class_unregister(gpu_class);
    class_destroy(gpu_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Módulo descarregado\n");

}


module_init(my_module_init);
module_exit(my_module_exit);

