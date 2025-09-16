/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * QEMU RISC-V Virt Board Compatible with kendryte K230 SDK
 *
 * Copyright (c) 2025 Chao Liu <chao.liu@zevorn.cn>
 * Copyright (c) 2025 Shengjie Lin <2874146120@qq.com>
 *
 * Provides a board compatible with the kendryte K230 SDK
 *
 * For more information, see <https://www.kendryte.com/en/proDetail/230>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "qemu/cutils.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "hw/boards.h"
#include "hw/loader.h"
#include "hw/sysbus.h"
#include "hw/misc/unimp.h"
#include "target/riscv/cpu.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/k230.h"
#include "hw/riscv/boot.h"
#include "hw/intc/riscv_aclint.h"
#include "hw/gpio/sifive_gpio.h"
#include "hw/intc/sifive_plic.h"
#include "hw/char/sifive_uart.h"
#include "chardev/char.h"
#include "system/cpus.h"
#include "system/system.h"

static const MemMapEntry k230_memmap[] = {
    [K230_DEV_DDRC] =            { 0x00000000,   0x80000000 }, /* 2GB */
    [K230_DEV_KPU_L2_CACHE] =    { 0x80000000,     0x200000 }, /* 2MB */
    [K230_DEV_SRAM] =            { 0x80200000,     0x200000 }, /* 2MB */
    [K230_DEV_KPU_CFG] =         { 0x80400000,        0x800 }, /* 2KB */
    [K230_DEV_FFT] =             { 0x80400800,        0x400 }, /* 1KB */
    [K230_DEV_AI_2D_ENGINE] =    { 0x80400C00,        0x400 }, /* 1KB */
    [K230_DEV_GSDMA] =           { 0x80800000,       0x4000 }, /* 16KB */
    [K230_DEV_DMA] =             { 0x80804000,       0x4000 }, /* 16KB */
    [K230_DEV_DECOMP_GZIP] =     { 0x80808000,       0x4000 }, /* 16KB */
    [K230_DEV_NON_AI_2D] =       { 0x8080C000,       0x4000 }, /* 16KB */
    [K230_DEV_ISP] =             { 0x90000000,       0x8000 }, /* 32KB */
    [K230_DEV_DEWARP] =          { 0x90008000,       0x1000 }, /* 4KB */
    [K230_DEV_RX_CSI] =          { 0x90009000,       0x2000 }, /* 8KB */
    [K230_DEV_H264] =            { 0x90400000,      0x10000 }, /* 64KB */
    [K230_DEV_2P5D] =            { 0x90800000,      0x40000 }, /* 256KB */
    [K230_DEV_VO] =              { 0x90840000,      0x10000 }, /* 64KB */
    [K230_DEV_VO_CFG] =          { 0x90850000,       0x1000 }, /* 4KB */
    [K230_DEV_3D_ENGINE] =       { 0x90A00000,        0x800 }, /* 2KB */
    [K230_DEV_PMU] =             { 0x91000000,        0xC00 }, /* 3KB */
    [K230_DEV_RTC] =             { 0x91000C00,        0x400 }, /* 1KB */
    [K230_DEV_CMU] =             { 0x91100000,       0x1000 }, /* 4KB */
    [K230_DEV_RMU] =             { 0x91101000,       0x1000 }, /* 4KB */
    [K230_DEV_BOOT] =            { 0x91102000,       0x1000 }, /* 4KB */
    [K230_DEV_PWR] =             { 0x91103000,       0x1000 }, /* 4KB */
    [K230_DEV_MAILBOX] =         { 0x91104000,       0x1000 }, /* 4KB */
    [K230_DEV_IOMUX] =           { 0x91105000,        0x800 }, /* 2KB */
    [K230_DEV_TIMER] =           { 0x91105800,        0x800 }, /* 2KB */
    [K230_DEV_WDT0] =            { 0x91106000,        0x800 }, /* 2KB */
    [K230_DEV_WDT1] =            { 0x91106800,        0x800 }, /* 2KB */
    [K230_DEV_TS] =              { 0x91107000,        0x800 }, /* 2KB */
    [K230_DEV_HDI] =             { 0x91107800,        0x800 }, /* 2KB */
    [K230_DEV_STC] =             { 0x91108000,       0x1000 }, /* 4KB */
    [K230_DEV_BOOTROM] =         { 0x91200000,      0x10000 }, /* 64KB */
    [K230_DEV_SECURITY] =        { 0x91210000,       0x8000 }, /* 32KB */
    [K230_DEV_UART0] =           { 0x91400000,       0x1000 }, /* 4KB */
    [K230_DEV_UART1] =           { 0x91401000,       0x1000 }, /* 4KB */
    [K230_DEV_UART2] =           { 0x91402000,       0x1000 }, /* 4KB */
    [K230_DEV_UART3] =           { 0x91403000,       0x1000 }, /* 4KB */
    [K230_DEV_UART4] =           { 0x91404000,       0x1000 }, /* 4KB */
    [K230_DEV_I2C0] =            { 0x91405000,       0x1000 }, /* 4KB */
    [K230_DEV_I2C1] =            { 0x91406000,       0x1000 }, /* 4KB */
    [K230_DEV_I2C2] =            { 0x91407000,       0x1000 }, /* 4KB */
    [K230_DEV_I2C3] =            { 0x91408000,       0x1000 }, /* 4KB */
    [K230_DEV_I2C4] =            { 0x91409000,       0x1000 }, /* 4KB */
    [K230_DEV_PWM] =             { 0x9140A000,       0x1000 }, /* 4KB */
    [K230_DEV_GPIO0] =           { 0x9140B000,       0x1000 }, /* 4KB */
    [K230_DEV_GPIO1] =           { 0x9140C000,       0x1000 }, /* 4KB */
    [K230_DEV_ADC] =             { 0x9140D000,       0x1000 }, /* 4KB */
    [K230_DEV_CODEC] =           { 0x9140E000,       0x1000 }, /* 4KB */
    [K230_DEV_AUDIO] =           { 0x9140F000,       0x1000 }, /* 4KB */
    [K230_DEV_USB] =             { 0x91500000,      0x80000 }, /* 512KB */
    [K230_DEV_SD] =              { 0x91580000,       0x2000 }, /* 8KB */
    [K230_DEV_SPI_QOPI] =        { 0x91582000,       0x2000 }, /* 8KB */
    [K230_DEV_SPI_OPI] =         { 0x91584000,       0x1000 }, /* 4KB */
    [K230_DEV_HI_SYS_CONFIG] =   { 0x91585000,        0x400 }, /* 1KB */
    [K230_DEV_DDRC_CONFIG] =     { 0x98000000,    0x2000000 }, /* 32MB */
    [K230_DEV_FLASH] =           { 0xC0000000,    0x8000000 }, /* 128MB */
    [K230_DEV_CLINT] =           { 0x02000000,      0x10000 }, /* fixme */
    [K230_DEV_PLIC] =            { 0xf0000000,     0x200000 }, /* 2MB */
};

static void k230_machine_init(MachineState *machine)
{
    MachineClass *mc = MACHINE_GET_CLASS(machine);
    const MemMapEntry *memmap = k230_memmap;

    K230State *s = RISCV_K230_MACHINE(machine);
    MemoryRegion *sys_mem = get_system_memory();
    RISCVBootInfo boot_info;

    if (machine->ram_size != mc->default_ram_size) {
        char *sz = size_to_str(mc->default_ram_size);
        error_report("Invalid RAM size, should be %s", sz);
        g_free(sz);
        exit(EXIT_FAILURE);
    }

    /* Initialize SoC */
    object_initialize_child(OBJECT(machine), "soc", &s->soc,
                            TYPE_RISCV_K230_SOC);
    qdev_realize(DEVICE(&s->soc), NULL, &error_fatal);

    /* Data Memory(DDR RAM) */
    memory_region_add_subregion(sys_mem,
        memmap[K230_DEV_DDRC].base, machine->ram);

    /* Mask ROM reset vector */
    uint32_t bootrom_code[3];

    bootrom_code[0] = 0x000002b7; /* 0x91200000: lui   t0, 0x0 */
    bootrom_code[1] = 0x00028067; /* 0x91200004: jr    t0 */
    bootrom_code[2] = 0;

    /* copy in the reset vector in little_endian byte order */
    for (int i = 0; i < sizeof(bootrom_code) >> 2; i++) {
        bootrom_code[i] = cpu_to_le32(bootrom_code[i]);
    }
    rom_add_blob_fixed_as("bootrom", bootrom_code, sizeof(bootrom_code),
                         memmap[K230_DEV_BOOTROM].base, &address_space_memory);

    /* 加载内核/Opensbi */
    riscv_boot_info_init(&boot_info, &s->soc.c908_cpus);
    if (machine->kernel_filename) {
        riscv_load_kernel(machine, &boot_info,
                          memmap[K230_DEV_DDRC].base,
                          false, NULL);
    }
}

static void k230_machine_class_init(ObjectClass *oc, const void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    static const char *const cpu_types[] = {
      K230_C908_CPU, K230_C908V_CPU, NULL };

    mc->desc = "RISC-V Board compatible with K230";
    mc->init = k230_machine_init;
    mc->max_cpus = 2;
    mc->valid_cpu_types = cpu_types;
    mc->default_cpus = 1;
    mc->default_cpu_type = K230_C908_CPU;
    mc->default_ram_id = "riscv.k230.ram";
    mc->default_ram_size = k230_memmap[K230_DEV_DDRC].size;
}

static void k230_machine_instance_init(Object *obj)
{
}

static const TypeInfo k230_machine_typeinfo = {
    .name       = MACHINE_TYPE_NAME("k230"),
    .parent     = TYPE_MACHINE,
    .class_init = k230_machine_class_init,
    .instance_init = k230_machine_instance_init,
    .instance_size = sizeof(K230State),
};

static void k230_machine_init_register_types(void)
{
    type_register_static(&k230_machine_typeinfo);
}

type_init(k230_machine_init_register_types)

static void k230_soc_init(Object *obj)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    K230SoCState *s = RISCV_K230_SOC(obj);

    if (ms->smp.cpus > 2) {
        error_report("K230 supports at most 2 CPUs (1xC908 + 1xC908V)");
        exit(1);
    }

    object_initialize_child(obj, "c908-cpus", &s->c908_cpus,
                            TYPE_RISCV_HART_ARRAY);
    object_property_set_int(OBJECT(&s->c908_cpus), "num-harts", 1,
                            &error_abort);
    object_property_set_int(OBJECT(&s->c908_cpus), "resetvec",
                            k230_memmap[K230_DEV_BOOTROM].base,
                            &error_abort);

    if (ms->smp.cpus == 2) {
        object_initialize_child(obj, "c908v-cpus", &s->c908v_cpus,
                                TYPE_RISCV_HART_ARRAY);
        object_property_set_int(OBJECT(&s->c908v_cpus), "num-harts", 1,
                                &error_abort);
        object_property_set_int(OBJECT(&s->c908v_cpus), "resetvec",
                                k230_memmap[K230_DEV_BOOTROM].base,
                                &error_abort);
    }

    /* gpio */
    object_initialize_child(obj, "riscv.k230.gpio0", &s->gpio0,
                            TYPE_SIFIVE_GPIO);
    object_property_set_int(OBJECT(&s->gpio0), "ngpio", 32,
                            &error_abort);
    object_initialize_child(obj, "riscv.k230.gpio1", &s->gpio1,
                            TYPE_SIFIVE_GPIO);
    object_property_set_int(OBJECT(&s->gpio1), "ngpio", 32,
                            &error_abort);

}

static void k230_soc_realize(DeviceState *dev, Error **errp)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    const MemMapEntry *memmap = k230_memmap;
    K230SoCState *s = RISCV_K230_SOC(dev);
    MemoryRegion *sys_mem = get_system_memory();

    /* CPUs realize */
    object_property_set_str(OBJECT(&s->c908_cpus), "cpu-type", ms->cpu_type,
                            &error_abort);
    sysbus_realize(SYS_BUS_DEVICE(&s->c908_cpus), &error_fatal);

    if (ms->smp.cpus == 2) {
      object_property_set_str(OBJECT(&s->c908v_cpus), "cpu-type",
                              ms->cpu_type, &error_abort);
        sysbus_realize(SYS_BUS_DEVICE(&s->c908v_cpus), &error_fatal);
    }

    /* PLIC */
    const char *hart_config = ms->smp.cpus == 2 ? "MS,MS" : "MS";
    s->plic = sifive_plic_create(memmap[K230_DEV_PLIC].base,
        (char *)hart_config, ms->smp.cpus, 0,
        K230_PLIC_NUM_SOURCES,
        K230_PLIC_NUM_PRIORITIES,
        K230_PLIC_PRIORITY_BASE,
        K230_PLIC_PENDING_BASE,
        K230_PLIC_ENABLE_BASE,
        K230_PLIC_ENABLE_STRIDE,
        K230_PLIC_CONTEXT_BASE,
        K230_PLIC_CONTEXT_STRIDE,
        memmap[K230_DEV_PLIC].size);

    /* 将 PLIC 输出连到两个 CPU 的 IRQ_M_EXT */
    for (int hart = 0; hart < ms->smp.cpus; hart++) {
        if (hart == 0) {
            qdev_connect_gpio_out(DEVICE(s->plic), hart,
                qdev_get_gpio_in(DEVICE(&s->c908_cpus.harts[0]), IRQ_M_EXT));
        } else if (hart == 1) {
            qdev_connect_gpio_out(DEVICE(s->plic), hart,
                    qdev_get_gpio_in(DEVICE(&s->c908v_cpus.harts[0]),
                                                   IRQ_M_EXT));
        }
    }

    hwaddr clint_base = memmap[K230_DEV_CLINT].base;
    s->clint = DEVICE(riscv_aclint_swi_create(
        clint_base, 0, ms->smp.cpus, false));
    if (!s->clint) {
        error_report("k230: riscv_aclint_swi_create failed");
        return;
    }
    riscv_aclint_mtimer_create(
        clint_base + RISCV_ACLINT_SWI_SIZE,
        RISCV_ACLINT_DEFAULT_MTIMER_SIZE,
        0, ms->smp.cpus,
        RISCV_ACLINT_DEFAULT_MTIMECMP,
        RISCV_ACLINT_DEFAULT_MTIME,
        K230_LFCLK_DEFAULT_FREQ,
        false);

    /* GPIO */
    if (!sysbus_realize(SYS_BUS_DEVICE(&s->gpio0), errp)) {
        return;
    }
    if (!sysbus_realize(SYS_BUS_DEVICE(&s->gpio1), errp)) {
        return;
    }

    /* Map GPIO registers */
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->gpio0), 0,
                    memmap[K230_DEV_GPIO0].base);
    sysbus_mmio_map(SYS_BUS_DEVICE(&s->gpio1), 0,
                    memmap[K230_DEV_GPIO1].base);

    /* Pass all GPIOs to the SOC layer so they are available to the board */
    qdev_pass_gpios(DEVICE(&s->gpio0), dev, "gpio0");
    qdev_pass_gpios(DEVICE(&s->gpio1), dev, "gpio1");

    /* Connect GPIO interrupts to the PLIC */
    for (int i = 0; i < 32; i++) {
        sysbus_connect_irq(SYS_BUS_DEVICE(&s->gpio0), i,
                           qdev_get_gpio_in(DEVICE(s->plic),
                                            K230_GPIO0_IRQ0 + i));
        sysbus_connect_irq(SYS_BUS_DEVICE(&s->gpio1), i,
                           qdev_get_gpio_in(DEVICE(s->plic),
                                            K230_GPIO0_IRQ0 + 32 + i));
    }

    /* UART */
    for (int i = 0; i < K230_UART_IRQS; i++) {
        hwaddr base = memmap[K230_DEV_UART0 + i].base;
        int irq = K230_UART0_IRQ + i;

        s->uart[i] = DEVICE(sifive_uart_create(
            sys_mem, base,
            serial_hd(i),
            qdev_get_gpio_in(DEVICE(s->plic), irq)
        ));
    }

    /* PWM */
    create_unimplemented_device("riscv.k230.pwm0", memmap[K230_DEV_PWM].base,
                                memmap[K230_DEV_PWM].size);
}

static void k230_soc_class_init(ObjectClass *oc, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = k230_soc_realize;
    /* Reason: Uses serial_hds in realize function, thus can't be used twice */
    dc->user_creatable = false;
}

static const TypeInfo k230_soc_type_info = {
    .name = TYPE_RISCV_K230_SOC,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(K230SoCState),
    .instance_init = k230_soc_init,
    .class_init = k230_soc_class_init,
};

static void k230_soc_register_types(void)
{
    type_register_static(&k230_soc_type_info);
}

type_init(k230_soc_register_types)
