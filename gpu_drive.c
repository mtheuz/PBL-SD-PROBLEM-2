#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/io.h>

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

// Declaração de variáveis globais
static int major_number;
static struct class* gpu_class = NULL;
static struct device* gpu_device = NULL;
static struct cdev gpu_cdev;

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
    iowrite32(0, START_PTR);  // Zerar o registrador de início
    iowrite32(opcode, DATA_A_PTR);  // Escrever o código de operação no registrador de dados A
    iowrite32(data, DATA_B_PTR);  // Escrever os dados no registrador de dados B
    iowrite32(1, START_PTR);  // Sinalizar início da operação para a GPU
    iowrite32(0, START_PTR);  // Limpar o sinal de início após a operação
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

void instrucao_wbm(int address, int r, int g, int b) {
    volatile int opcode = WBM; // Opcode para WBM
    volatile int dados = (b << 16) | (g << 8) | r;
    volatile int opcode_reg = (address << 14) | opcode;
    send_instruction(opcode_reg, dados);
}

void instrucao_wsm(int address, int r, int g, int b) {
    volatile int opcode = WSM; // Opcode para WSM
    volatile int dados = (b << 16) | (g << 8) | r;
    volatile int opcode_reg = (address << 16) | opcode;
    send_instruction(opcode_reg, dados);
}

//terminar
void instrucao_dp(int address, int ref_x, int ref_y, int size, int r, int g, int b, int shape) {
    int opcode = DP; // Opcode para DP
    int instruction = (opcode << 28) | (address << 24) | (ref_x << 16) | (ref_y << 8) | size;
    instruction |= (r << 16) | (g << 8) | b;
    if (shape) {
        instruction |= (1 << 31);
    }
    send_instruction(instruction, 0); // Assumindo que 'dados' não é usado para DP
}

static int device_open(struct inode *inodep, struct file *filep) {
    return 0;
}

static int device_release(struct inode *inodep, struct file *filep) {
    return 0;
}

static ssize_t device_write(struct file *filep, const char *buffer, size_t len) {
    unsigned char command[9]; 

    if (len < 4 || len > 9) {
        printk(KERN_ALERT "Comprimento de comando inválido\n");
        return -EINVAL;
    }

    if (copy_from_user(command, buffer, 9)) {
        return -EFAULT;
    }

    switch (command[0]) {
        case "WBR":
            instrucao_wbr(command[1], command[2], command[3], command[4], command[5], command[6], command[7]);
            break;
        case "WBM":
            instrucao_wbm(command[1], command[2], command[3], command[4]);
            break;
        case "WSM":
            instrucao_wsm(command[1], command[2], command[3], command[4]);
            break;
        case "DP":
            instrucao_dp(command[1], command[2], command[3], command[4], command[5], command[6], command[7], command[8]);
            break;
        default:
            printk(KERN_ALERT "Comando desconhecido\n");
            return -EINVAL;
    }

    return len;
}

static int __init module_init(void) {
    LW_virtual = ioremap(LW_BRIDGE_BASE, LW_BRIDGE_SPAN);
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
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
    
    printk(KERN_INFO "Módulo carregado: classe do dispositivo criada corretamente\n");
    return 0;
}

static void __exit my_module_exit(void) {
    iounmap(LW_virtual);
    device_destroy(gpu_class, MKDEV(major_number, 0));
    class_unregister(gpu_class);
    class_destroy(gpu_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Módulo descarregado\n");

module_init(my_module_init);
module_exit(my_module_exit);

}


