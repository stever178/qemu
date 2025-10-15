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
#include "hw/char/serial-mm.h"
#include "hw/misc/unimp.h"
#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/k230.h"
#include "hw/riscv/boot.h"
#include "hw/intc/riscv_aclint.h"
#include "hw/gpio/sifive_gpio.h"
#include "hw/intc/sifive_plic.h"
#include "hw/char/sifive_uart.h"
#include "chardev/char.h"
#include "system/cpus.h"
#include "system/device_tree.h"
#include "system/system.h"

static const MemMapEntry k230_memmap[] = {
    [K230_DEV_DDRC] =         { 0x00000000, 0x80000000 },
    [K230_DEV_KPU_L2_CACHE] = { 0x80000000, 0x00200000 },
    [K230_DEV_SRAM] =         { 0x80200000, 0x00200000 },
    [K230_DEV_KPU_CFG] =      { 0x80400000, 0x00000800 },
    [K230_DEV_FFT] =          { 0x80400800, 0x00000400 },
    [K230_DEV_AI_2D_ENGINE] = { 0x80400C00, 0x00000800 },
    [K230_DEV_GSDMA] =        { 0x80800000, 0x00004000 },
    [K230_DEV_DMA] =          { 0x80804000, 0x00004000 },
    [K230_DEV_DECOMP_GZIP] =  { 0x80808000, 0x00004000 },
    [K230_DEV_NON_AI_2D] =    { 0x8080C000, 0x00004000 },
    [K230_DEV_ISP] =          { 0x90000000, 0x00008000 },
    [K230_DEV_DEWARP] =       { 0x90008000, 0x00001000 },
    [K230_DEV_RX_CSI] =       { 0x90009000, 0x00002000 },
    [K230_DEV_H264] =         { 0x90400000, 0x00010000 },
    [K230_DEV_2P5D] =         { 0x90800000, 0x00040000 },
    [K230_DEV_VO] =           { 0x90840000, 0x00010000 },
    [K230_DEV_VO_CFG] =       { 0x90850000, 0x00001000 },
    [K230_DEV_3D_ENGINE] =    { 0x90A00000, 0x00000800 },
    [K230_DEV_PMU] =          { 0x91000000, 0x00000C00 },
    [K230_DEV_RTC] =          { 0x91000C00, 0x00000400 },
    [K230_DEV_CMU] =          { 0x91100000, 0x00001000 },
    [K230_DEV_RMU] =          { 0x91101000, 0x00001000 },
    [K230_DEV_BOOT] =         { 0x91102000, 0x00001000 },
    [K230_DEV_PWR] =          { 0x91103000, 0x00001000 },
    [K230_DEV_MAILBOX] =      { 0x91104000, 0x00001000 },
    [K230_DEV_IOMUX] =        { 0x91105000, 0x00000800 },
    [K230_DEV_TIMER] =        { 0x91105800, 0x00000800 },
    [K230_DEV_WDT0] =         { 0x91106000, 0x00000800 },
    [K230_DEV_WDT1] =         { 0x91106800, 0x00000800 },
    [K230_DEV_TS] =           { 0x91107000, 0x00000800 },
    [K230_DEV_HDI] =          { 0x91107800, 0x00000800 },
    [K230_DEV_STC] =          { 0x91108000, 0x00000800 },
    [K230_DEV_BOOTROM] =      { 0x91200000, 0x00010000 },
    [K230_DEV_SECURITY] =     { 0x91210000, 0x00008000 },
    [K230_DEV_UART0] =        { 0x91400000, 0x00001000 },
    [K230_DEV_UART1] =        { 0x91401000, 0x00001000 },
    [K230_DEV_UART2] =        { 0x91402000, 0x00001000 },
    [K230_DEV_UART3] =        { 0x91403000, 0x00001000 },
    [K230_DEV_UART4] =        { 0x91404000, 0x00001000 },
    [K230_DEV_I2C0] =         { 0x91405000, 0x00001000 },
    [K230_DEV_I2C1] =         { 0x91406000, 0x00001000 },
    [K230_DEV_I2C2] =         { 0x91407000, 0x00001000 },
    [K230_DEV_I2C3] =         { 0x91408000, 0x00001000 },
    [K230_DEV_I2C4] =         { 0x91409000, 0x00001000 },
    [K230_DEV_PWM] =          { 0x9140A000, 0x00001000 },
    [K230_DEV_GPIO0] =        { 0x9140B000, 0x00001000 },
    [K230_DEV_GPIO1] =        { 0x9140C000, 0x00001000 },
    [K230_DEV_ADC] =          { 0x9140D000, 0x00001000 },
    [K230_DEV_CODEC] =        { 0x9140E000, 0x00001000 },
    [K230_DEV_I2S] =          { 0x9140F000, 0x00001000 },
    [K230_DEV_USB0] =         { 0x91500000, 0x00010000 },
    [K230_DEV_USB1] =         { 0x91540000, 0x00010000 },
    [K230_DEV_SD0] =          { 0x91580000, 0x00001000 },
    [K230_DEV_SD1] =          { 0x91581000, 0x00001000 },
    [K230_DEV_QSPI0] =        { 0x91582000, 0x00001000 },
    [K230_DEV_QSPI1] =        { 0x91583000, 0x00001000 },
    [K230_DEV_SPI] =          { 0x91584000, 0x00001000 },
    [K230_DEV_HI_SYS_CFG] =   { 0x91585000, 0x00000400 },
    [K230_DEV_DDRC_CFG] =     { 0x98000000, 0x02000000 },
    [K230_DEV_FLASH] =        { 0xC0000000, 0x08000000 },
    [K230_DEV_PLIC] =         { 0xF0000000, 0x00400000 },
    [K230_DEV_CLINT] =        { 0xF0400000, 0x00400000 },
};

static void create_fdt(K230State *s, const MemMapEntry *memmap)
{
    MachineState *ms = MACHINE(s);
    uint64_t mem_size = ms->ram_size;
    void *fdt;
    int cpu;
    uint32_t *cells;
    char *nodename;
    uint32_t plic_phandle, phandle = 1;
    uint32_t def_50mhz_phandle, rtcclk_phandle;
    static const char * const clint_compat[] = { "riscv,clint0" };
    static const char * const plic_compat[] = { "riscv,plic0" };

    if (ms->smp.cpus > 2) {
        error_report("K230 supports at most 2 CPUs (1xC908 + 1xC908V)");
        exit(1);
    }

    fdt = ms->fdt = create_device_tree(&s->fdt_size);
    if (!ms->fdt) {
        error_report("create_device_tree() failed");
        exit(1);
    }

    qemu_fdt_setprop_string(fdt, "/",
                            "model", "kendryte k230 canmv");
    qemu_fdt_setprop_string(fdt, "/", "compatible",
                            "kendryte,k230_canmv");
    qemu_fdt_setprop_cell(fdt, "/",
                          "#address-cells", 0x2);
    qemu_fdt_setprop_cell(fdt, "/",
                          "#size-cells", 0x2);

    qemu_fdt_add_subnode(fdt, "/soc");
    qemu_fdt_setprop(fdt, "/soc", "ranges",
                     NULL, 0);
    qemu_fdt_setprop_string(fdt, "/soc",
                            "compatible", "simple-bus");
    qemu_fdt_setprop_cell(fdt, "/soc",
                          "#size-cells", 0x2);
    qemu_fdt_setprop_cell(fdt, "/soc",
                          "#address-cells", 0x2);

    def_50mhz_phandle = phandle++;
    nodename = g_strdup_printf("/def_50mhz");
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_cell(fdt, nodename,
                          "phandle", def_50mhz_phandle);
    qemu_fdt_setprop_string(fdt, nodename,
                            "clock-output-names", "fix-50mhz");
    qemu_fdt_setprop_cell(fdt, nodename,
                          "clock-frequency", K230_FIX50M_FREQ);
    qemu_fdt_setprop_string(fdt, nodename,
                            "compatible", "fixed-clock");
    qemu_fdt_setprop_cell(fdt, nodename,
                          "#clock-cells", 0x0);
    g_free(nodename);

    rtcclk_phandle = phandle++;
    nodename = g_strdup_printf("/rtcclk");
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_cell(fdt, nodename,
                          "phandle", rtcclk_phandle);
    qemu_fdt_setprop_string(fdt, nodename,
                            "clock-output-names", "rtcclk");
    qemu_fdt_setprop_cell(fdt, nodename,
                          "clock-frequency", K230_RTCCLK_FREQ);
    qemu_fdt_setprop_string(fdt, nodename,
                            "compatible", "fixed-clock");
    qemu_fdt_setprop_cell(fdt, nodename,
                          "#clock-cells", 0x0);
    g_free(nodename);

    nodename = g_strdup_printf("/memory@%lx",
                               (long)memmap[K230_DEV_DDRC].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_string(fdt, nodename,
                            "device_type", "memory");
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
        memmap[K230_DEV_DDRC].base >> 32, memmap[K230_DEV_DDRC].base,
        mem_size >> 32, mem_size);
    g_free(nodename);

    qemu_fdt_add_subnode(fdt, "/cpus");
    qemu_fdt_setprop_cell(fdt, "/cpus",
                          "#address-cells", 0x1);
    qemu_fdt_setprop_cell(fdt, "/cpus",
                          "#size-cells", 0x0);
    qemu_fdt_setprop_cell(fdt, "/cpus",
                          "timebase-frequency",
                          K230_TIMEBASE_FREQ);

    for (cpu = 0; cpu < ms->smp.cpus; cpu++) {
        int cpu_phandle = phandle++;
        nodename = g_strdup_printf("/cpus/cpu@%d", cpu);
        char *intc = g_strdup_printf(
            "/cpus/cpu@%d/interrupt-controller", cpu);

        qemu_fdt_add_subnode(fdt, nodename);
        qemu_fdt_setprop_string(fdt, nodename,
                                "compatible", "riscv");

        if (cpu == 0) {
            qemu_fdt_setprop_string(fdt, nodename,
                                    "riscv,isa",
                                    "rv64imafdcvsu");
            qemu_fdt_setprop_string(fdt, nodename,
                                    "mmu-type", "riscv,sv39");
            qemu_fdt_setprop_cell(fdt, nodename,
                                  "clock-frequency",
                                  K230_CPUCLK_FREQ);

            riscv_isa_write_fdt(&s->soc.c908_cpu.harts[0], fdt, nodename);
        } else {
            qemu_fdt_setprop_string(fdt, nodename,
                                    "riscv,isa",
                                    "rv64imafdcvsu");
            qemu_fdt_setprop_string(fdt, nodename,
                                  "mmu-type", "riscv,sv39");
            qemu_fdt_setprop_cell(fdt, nodename,
                                  "clock-frequency",
                                  K230_CPUCLK_FREQ);
            riscv_isa_write_fdt(&s->soc.c908v_cpu.harts[0],
                                fdt, nodename);
        }

        qemu_fdt_setprop_string(fdt, nodename,
                                "status", "okay");
        qemu_fdt_setprop_cell(fdt, nodename,
                              "reg", cpu);
        qemu_fdt_setprop_string(fdt, nodename,
                                "device_type", "cpu");
        
        qemu_fdt_add_subnode(fdt, intc);
        qemu_fdt_setprop_cell(fdt, intc,
                              "phandle", cpu_phandle);
        qemu_fdt_setprop_string(fdt, intc, "compatible",
                                "riscv,cpu-intc");
        qemu_fdt_setprop(fdt, intc,
                         "interrupt-controller", NULL, 0);
        qemu_fdt_setprop_cell(fdt, intc,
                              "#interrupt-cells", 1);

        g_free(intc);
        g_free(nodename);
    }

    cells = g_new0(uint32_t, ms->smp.cpus * 4);
    for (cpu = 0; cpu < ms->smp.cpus; cpu++) {
        nodename =
            g_strdup_printf("/cpus/cpu@%d/interrupt-controller", cpu);
        uint32_t intc_phandle = qemu_fdt_get_phandle(fdt, nodename);
        cells[cpu * 4 + 0] = cpu_to_be32(intc_phandle);
        cells[cpu * 4 + 1] = cpu_to_be32(IRQ_M_SOFT);
        cells[cpu * 4 + 2] = cpu_to_be32(intc_phandle);
        cells[cpu * 4 + 3] = cpu_to_be32(IRQ_M_TIMER);
        g_free(nodename);
    }
    nodename = g_strdup_printf("/soc/clint@%lx",
        (long)memmap[K230_DEV_CLINT].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_string_array(fdt, nodename, "compatible",
        (char **)&clint_compat, ARRAY_SIZE(clint_compat));
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
        0x0, memmap[K230_DEV_CLINT].base,
        0x0, memmap[K230_DEV_CLINT].size);
    qemu_fdt_setprop(fdt, nodename, "interrupts-extended",
                     cells, ms->smp.cpus * 4 * sizeof(uint32_t));
    g_free(cells);
    g_free(nodename);
    
    plic_phandle = phandle++;
    cells = g_new0(uint32_t, ms->smp.cpus * 2);
    for (cpu = 0; cpu < ms->smp.cpus; cpu++) {
        nodename =
            g_strdup_printf("/cpus/cpu@%d/interrupt-controller", cpu);
        uint32_t intc_phandle = qemu_fdt_get_phandle(fdt, nodename);
        cells[cpu * 2 + 0] = cpu_to_be32(intc_phandle);
        cells[cpu * 2 + 1] = cpu_to_be32(IRQ_M_EXT);
        g_free(nodename);
    }
    nodename = g_strdup_printf("/soc/interrupt-controller@%lx",
        (long)memmap[K230_DEV_PLIC].base);
    qemu_fdt_add_subnode(fdt, nodename);
    qemu_fdt_setprop_cell(fdt, nodename,
                          "#interrupt-cells", 1);
    qemu_fdt_setprop_string_array(fdt, nodename, "compatible",
        (char **)&plic_compat, ARRAY_SIZE(plic_compat));
    qemu_fdt_setprop(fdt, nodename,
                     "interrupt-controller", NULL, 0);
    qemu_fdt_setprop(fdt, nodename,
                     "interrupts-extended",
        cells, (ms->smp.cpus * 2) * sizeof(uint32_t));
    qemu_fdt_setprop_cells(fdt, nodename, "reg",
        0x0, memmap[K230_DEV_PLIC].base,
        0x0, memmap[K230_DEV_PLIC].size);
    qemu_fdt_setprop_cell(fdt, nodename, "riscv,ndev",
                          K230_PLIC_NUM_SOURCES - 1);
    qemu_fdt_setprop_cell(fdt, nodename,
                          "phandle", plic_phandle);
    plic_phandle = qemu_fdt_get_phandle(fdt, nodename);
    g_free(cells);
    g_free(nodename);

    for (int i = 0; i < K230_UART_IRQS; i++) {
        nodename = g_strdup_printf("/soc/serial@%lx",
            (long)memmap[K230_DEV_UART0 + i].base);
        qemu_fdt_add_subnode(fdt, nodename);

        qemu_fdt_setprop_string(fdt, nodename, "compatible",
                                "snps,dw-apb-uart");
        qemu_fdt_setprop_cells(fdt, nodename, "reg",
            0x0, memmap[K230_DEV_UART0 + i].base,
            0x0, 0x400);
        qemu_fdt_setprop_cells(fdt, nodename,
                               "clocks", def_50mhz_phandle);
        qemu_fdt_setprop_string(fdt, nodename,
                                "clock-names", "baudclk");
        qemu_fdt_setprop_cell(fdt, nodename,
                              "reg-shift", 2);
        qemu_fdt_setprop_cell(fdt, nodename,
                              "reg-io-width", 4);

        qemu_fdt_setprop_cell(fdt, nodename,
                              "interrupts", K230_UART0_IRQ + i);
        qemu_fdt_setprop_cell(fdt, nodename,
                              "interrupt-parent", plic_phandle);

        g_free(nodename);
    }

    qemu_fdt_add_subnode(fdt, "/aliases");
    qemu_fdt_setprop_string(fdt, "/aliases", "uart0",
                            "/soc/serial@91400000");

    qemu_fdt_add_subnode(fdt, "/chosen");
    qemu_fdt_setprop_string(fdt, "/chosen", "bootargs", 
        "console=ttyS0,115200n8 debug loglevel=7");
    qemu_fdt_setprop_string(fdt, "/chosen", "stdout-path",
                            "uart0:115200n8");
}

static void k230_machine_init(MachineState *machine)
{
    const MemMapEntry *memmap = k230_memmap;
    K230State *s = RISCV_K230_MACHINE(machine);
    MemoryRegion *sys_mem = get_system_memory();
    hwaddr start_addr = memmap[K230_DEV_DDRC].base;
    target_ulong firmware_end_addr, kernel_start_addr;
    const char *firmware_name;
    uint64_t fdt_load_addr;
    uint64_t kernel_entry;
    RISCVBootInfo boot_info;

    uint32_t mem_size = machine->ram_size;

    /* Initialize SoC */
    object_initialize_child(OBJECT(machine), "soc", &s->soc,
                            TYPE_RISCV_K230_SOC);
    object_property_set_str(OBJECT(&s->soc),
                            "cpu-type", machine->cpu_type,
                            &error_abort);
    qdev_realize(DEVICE(&s->soc), NULL, &error_fatal);

    /* Data Memory(DDR RAM) */
    memory_region_init_ram(machine->ram, NULL, "k230.dram",
                           mem_size, &error_fatal);
    memory_region_add_subregion(sys_mem,
        memmap[K230_DEV_DDRC].base, machine->ram);

    /* load/create device tree */
    if (machine->dtb) {
        machine->fdt = load_device_tree(machine->dtb,
                                      &s->fdt_size);
        if (!machine->fdt) {
            error_report("load_device_tree() failed");
            exit(1);
        }
    } else {
        create_fdt(s, k230_memmap);
    }

    firmware_name = riscv_default_firmware_name(&s->soc.c908_cpu);
    firmware_end_addr = riscv_find_and_load_firmware(
        machine, firmware_name,
        &start_addr, NULL);

    riscv_boot_info_init(&boot_info, &s->soc.c908_cpu);
    if (machine->kernel_filename) {
        kernel_start_addr = riscv_calc_kernel_start_addr(&boot_info,
                                                         firmware_end_addr);
        riscv_load_kernel(machine, &boot_info, kernel_start_addr,
                          true, NULL);
        kernel_entry = boot_info.image_low_addr;
    } else {
       /*
        * If dynamic firmware is used, it doesn't know where is the next mode
        * if kernel argument is not set.
        */
        kernel_entry = 0;
    }

    fdt_load_addr = riscv_compute_fdt_addr(memmap[K230_DEV_DDRC].base,
                                           mem_size,
                                           machine, &boot_info);
    riscv_load_fdt(fdt_load_addr, machine->fdt);

    /* Mask ROM reset vector */
    uint32_t reset_vec[] = {
        /* 0x91200000: auipc  t0, 0x0              */ 0x00000297,
        /* 0x91200004: addi   t0, t0, 36 # <trap>  */ 0x02428293,
        /* 0x91200008: csrw   mtvec, t0            */ 0x30529073,
        /* 0x9120000C: csrr   a0, mhartid          */ 0xf1402573,
        /* 0x91200010: beqz   a0, 9120000C <entry> */ 0x00050463,
        /* loop:                                   */
        /* 0x91200014: j      91200014 # <loop>    */ 0x0000006f,
        /* entry:                                  */
        /* 0x91200018: addiw  t0, zero, 1          */ 0x0010029b,
        /* 0x9120001C: slli   t0, t0, 0x1b         */ 0x01b29293,
        /* 0x91200020: jr     t0 # uboot 0x8000000 */ 0x00028067,
        /* trap:                                   */
        /* 0x91200024: j      91200024 # <trap>    */ 0x0000006f,
    };

    /* copy in the reset vector in little_endian byte order */
    for (int i = 0; i < ARRAY_SIZE(reset_vec); i++) {
        reset_vec[i] = cpu_to_le32(reset_vec[i]);
    }
    rom_add_blob_fixed_as("bootrom", reset_vec,
                          sizeof(reset_vec),
                          memmap[K230_DEV_BOOTROM].base,
                          &address_space_memory);

    riscv_rom_copy_firmware_info(machine, &s->soc.c908_cpu,
                                 memmap[K230_DEV_BOOTROM].base,
                                 memmap[K230_DEV_BOOTROM].size,
                                 sizeof(reset_vec), 
                                 kernel_entry);
}

static void k230_machine_class_init(ObjectClass *oc, const void *data)
{
    MachineClass *mc = MACHINE_CLASS(oc);
    static const char *const cpu_types[] = {
      K230_C908_CPU, K230_C908V_CPU, NULL };

    mc->desc = "RISC-V Board compatible with kendryte K230 SDK";
    mc->init = k230_machine_init;

    mc->max_cpus = 2;
    mc->valid_cpu_types = cpu_types;

    mc->default_cpus = 2;
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

static void k230_soc_instance_init(Object *obj)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    if (ms->smp.cpus > 2) {
        error_report("K230 supports at most 2 CPUs (1xC908 + 1xC908V)");
        exit(1);
    }

    K230SoCState *s = RISCV_K230_SOC(obj);

    object_initialize_child(obj, "c908-cpus", &s->c908_cpu,
                            TYPE_RISCV_HART_ARRAY);
    qdev_prop_set_uint32(DEVICE(&s->c908_cpu), "num-harts",
                         1);
    qdev_prop_set_uint32(DEVICE(&s->c908_cpu), "hartid-base",
                         CPU0_BASE_HARTID);
    qdev_prop_set_string(DEVICE(&s->c908_cpu), "cpu-type",
                         K230_C908_CPU);
    qdev_prop_set_uint64(DEVICE(&s->c908_cpu), "resetvec",
                         k230_memmap[K230_DEV_BOOTROM].base);

    if (ms->smp.cpus == 2) {
        object_initialize_child(obj, "c908v-cpus", &s->c908v_cpu,
                                TYPE_RISCV_HART_ARRAY);
        qdev_prop_set_uint32(DEVICE(&s->c908v_cpu),
                             "num-harts", 1);
        qdev_prop_set_uint32(DEVICE(&s->c908v_cpu),
                             "hartid-base", CPU1_BASE_HARTID);
        qdev_prop_set_string(DEVICE(&s->c908v_cpu),
                             "cpu-type", K230_C908V_CPU);
        qdev_prop_set_uint64(DEVICE(&s->c908v_cpu), "resetvec",
                             k230_memmap[K230_DEV_BOOTROM].base);
    }

    /* gpio */
    object_initialize_child(obj, "sifive.gpio0", &s->gpio0, TYPE_SIFIVE_GPIO);
    object_property_set_int(OBJECT(&s->gpio0), "ngpio",
                            K230_GPIO_IRQS,
                            &error_abort);
    object_initialize_child(obj, "sifive.gpio1", &s->gpio1, TYPE_SIFIVE_GPIO);
    object_property_set_int(OBJECT(&s->gpio1), "ngpio",
                            K230_GPIO_IRQS,
                            &error_abort);
}

static void k230_soc_realize(DeviceState *dev, Error **errp)
{
    MachineState *ms = MACHINE(qdev_get_machine());
    K230SoCState *s = RISCV_K230_SOC(dev);
    int num_harts = ms->smp.cpus;

    const MemMapEntry *memmap = k230_memmap;
    MemoryRegion *system_memory = get_system_memory();
    char *plic_hart_config;

    /* CPUs realize */
    sysbus_realize(SYS_BUS_DEVICE(&s->c908_cpu), &error_fatal);
    if (num_harts == 2) {
        sysbus_realize(SYS_BUS_DEVICE(&s->c908v_cpu), &error_fatal);
    }

    /* SRAM */
    memory_region_init_ram(&s->sram, OBJECT(dev), "sram",
                           memmap[K230_DEV_SRAM].size, &error_fatal);
    memory_region_add_subregion(system_memory,
                                memmap[K230_DEV_SRAM].base,
                                &s->sram);

    /* boot rom */
    memory_region_init_rom(&s->bootrom, OBJECT(dev), "bootrom",
                           memmap[K230_DEV_BOOTROM].size,
                           &error_fatal);
    memory_region_add_subregion(system_memory,
                                memmap[K230_DEV_BOOTROM].base,
                                &s->bootrom);

    /* create PLIC hart topology configuration string */
    plic_hart_config = riscv_plic_hart_config_string(num_harts);

    /* MMIO */
    s->plic = sifive_plic_create(memmap[K230_DEV_PLIC].base,
        plic_hart_config, num_harts, CPU0_BASE_HARTID,
        K230_PLIC_NUM_SOURCES,
        K230_PLIC_NUM_PRIORITIES,
        K230_PLIC_PRIORITY_BASE,
        K230_PLIC_PENDING_BASE,
        K230_PLIC_ENABLE_BASE,
        K230_PLIC_ENABLE_STRIDE,
        K230_PLIC_CONTEXT_BASE,
        K230_PLIC_CONTEXT_STRIDE,
        memmap[K230_DEV_PLIC].size);
    g_free(plic_hart_config);

    /* 将 PLIC 输出连到两个 CPU 的 IRQ_M_EXT */
    for (int hart = 0; hart < num_harts; hart++) {
        if (hart == 0) {
            qdev_connect_gpio_out(DEVICE(s->plic), hart,
                qdev_get_gpio_in(DEVICE(&s->c908_cpu.harts[0]),
                IRQ_M_EXT));
        } else if (hart == 1) {
            qdev_connect_gpio_out(
                DEVICE(s->plic), hart,
                qdev_get_gpio_in(DEVICE(&s->c908v_cpu.harts[0]),
                IRQ_M_EXT));
        }
    }

    hwaddr clint_base = memmap[K230_DEV_CLINT].base;
    riscv_aclint_swi_create(
        clint_base, CPU0_BASE_HARTID, num_harts, false);
    riscv_aclint_mtimer_create(
        clint_base + RISCV_ACLINT_SWI_SIZE,
        RISCV_ACLINT_DEFAULT_MTIMER_SIZE,
        CPU0_BASE_HARTID, num_harts,
        RISCV_ACLINT_DEFAULT_MTIMECMP,
        RISCV_ACLINT_DEFAULT_MTIME,
        K230_TIMEBASE_FREQ, /*fixme*/
        true);

    /* UART */
    for (int i = 0; i < K230_UART_IRQS; i++) {
        hwaddr base = memmap[K230_DEV_UART0 + i].base;
        int uart_irq = K230_UART0_IRQ + i;
        serial_mm_init(system_memory, base, 2,
                       qdev_get_gpio_in(DEVICE(s->plic), uart_irq),
                       K230_FIX50M_FREQ, /*fixme*/
                       serial_hd(i), DEVICE_LITTLE_ENDIAN);
    }

    /* other */
    create_unimplemented_device("kpu.l2-cache",
                                memmap[K230_DEV_KPU_L2_CACHE].base,
                                memmap[K230_DEV_KPU_L2_CACHE].size);

    create_unimplemented_device("kpu_cfg",
                                memmap[K230_DEV_KPU_CFG].base,
                                memmap[K230_DEV_KPU_CFG].size);

    create_unimplemented_device("fft",
        memmap[K230_DEV_FFT].base, memmap[K230_DEV_FFT].size);

    create_unimplemented_device("ai.2d-engine",
                                memmap[K230_DEV_AI_2D_ENGINE].base,
                                memmap[K230_DEV_AI_2D_ENGINE].size);

    create_unimplemented_device("gsdma",
        memmap[K230_DEV_GSDMA].base, memmap[K230_DEV_GSDMA].size);

    create_unimplemented_device("dma",
        memmap[K230_DEV_DMA].base, memmap[K230_DEV_DMA].size);

    create_unimplemented_device("decomp.gzip",
                                memmap[K230_DEV_DECOMP_GZIP].base,
                                memmap[K230_DEV_DECOMP_GZIP].size);

    create_unimplemented_device("non_ai.2d",
                                memmap[K230_DEV_NON_AI_2D].base,
                                memmap[K230_DEV_NON_AI_2D].size);

    create_unimplemented_device("isp",
        memmap[K230_DEV_ISP].base, memmap[K230_DEV_ISP].size);

    create_unimplemented_device("dewarp",
        memmap[K230_DEV_DEWARP].base, memmap[K230_DEV_DEWARP].size);

    create_unimplemented_device("rx_csi",
        memmap[K230_DEV_RX_CSI].base, memmap[K230_DEV_RX_CSI].size);

    create_unimplemented_device("h264",
        memmap[K230_DEV_H264].base, memmap[K230_DEV_H264].size);

    create_unimplemented_device("2p5d",
        memmap[K230_DEV_2P5D].base, memmap[K230_DEV_2P5D].size);

    create_unimplemented_device("vo",
        memmap[K230_DEV_VO].base, memmap[K230_DEV_VO].size);

    create_unimplemented_device("vo_cfg",
        memmap[K230_DEV_VO_CFG].base, memmap[K230_DEV_VO_CFG].size);

    create_unimplemented_device("3d_engine",
                                memmap[K230_DEV_3D_ENGINE].base,
                                memmap[K230_DEV_3D_ENGINE].size);

    create_unimplemented_device("pmu",
        memmap[K230_DEV_PMU].base, memmap[K230_DEV_PMU].size);

    create_unimplemented_device("rtc",
        memmap[K230_DEV_RTC].base, memmap[K230_DEV_RTC].size);

    create_unimplemented_device("cmu",
        memmap[K230_DEV_CMU].base, memmap[K230_DEV_CMU].size);

    create_unimplemented_device("rmu",
        memmap[K230_DEV_RMU].base, memmap[K230_DEV_RMU].size);

    create_unimplemented_device("boot",
        memmap[K230_DEV_BOOT].base, memmap[K230_DEV_BOOT].size);

    create_unimplemented_device("pwr",
        memmap[K230_DEV_PWR].base, memmap[K230_DEV_PWR].size);

    create_unimplemented_device("mailbox",
                                memmap[K230_DEV_MAILBOX].base,
                                memmap[K230_DEV_MAILBOX].size);

    create_unimplemented_device("iomux",
        memmap[K230_DEV_IOMUX].base, memmap[K230_DEV_IOMUX].size);

    create_unimplemented_device("timer",
        memmap[K230_DEV_TIMER].base, memmap[K230_DEV_TIMER].size);

    create_unimplemented_device("wdt0",
        memmap[K230_DEV_WDT0].base, memmap[K230_DEV_WDT0].size);

    create_unimplemented_device("wdt1",
        memmap[K230_DEV_WDT1].base, memmap[K230_DEV_WDT1].size);

    create_unimplemented_device("ts",
        memmap[K230_DEV_TS].base, memmap[K230_DEV_TS].size);

    create_unimplemented_device("hdi",
        memmap[K230_DEV_HDI].base, memmap[K230_DEV_HDI].size);

    create_unimplemented_device("stc",
        memmap[K230_DEV_STC].base, memmap[K230_DEV_STC].size);

    create_unimplemented_device("security",
                                memmap[K230_DEV_SECURITY].base,
                                memmap[K230_DEV_SECURITY].size);

    /* I2C */
    for (int i = 0; i < K230_I2C_IRQS; i++) {
        hwaddr base = memmap[K230_DEV_I2C0 + i].base;
        hwaddr size = memmap[K230_DEV_I2C0 + i].size;
        create_unimplemented_device("i2c", base, size);
    }

    /* PWM */
    create_unimplemented_device("pwm", memmap[K230_DEV_PWM].base,
                                memmap[K230_DEV_PWM].size);

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
    for (int i = 0; i < K230_GPIO_IRQS; i++) {
        sysbus_connect_irq(SYS_BUS_DEVICE(&s->gpio0), i,
                           qdev_get_gpio_in(DEVICE(s->plic),
                                            K230_GPIO0_IRQ0 + i));
        sysbus_connect_irq(SYS_BUS_DEVICE(&s->gpio1), i,
                           qdev_get_gpio_in(DEVICE(s->plic),
                                    K230_GPIO0_IRQ0 + K230_GPIO_IRQS + i));
    }

    /* other */
    create_unimplemented_device("adc",
        memmap[K230_DEV_ADC].base, memmap[K230_DEV_ADC].size);

    create_unimplemented_device("codec",
        memmap[K230_DEV_CODEC].base, memmap[K230_DEV_CODEC].size);

    create_unimplemented_device("i2s",
        memmap[K230_DEV_I2S].base, memmap[K230_DEV_I2S].size);

    create_unimplemented_device("usb0",
        memmap[K230_DEV_USB0].base, memmap[K230_DEV_USB0].size);

    create_unimplemented_device("usb1",
        memmap[K230_DEV_USB1].base, memmap[K230_DEV_USB1].size);

    create_unimplemented_device("sd0",
        memmap[K230_DEV_SD0].base, memmap[K230_DEV_SD0].size);

    create_unimplemented_device("sd1",
        memmap[K230_DEV_SD1].base, memmap[K230_DEV_SD1].size);

    create_unimplemented_device("qopi0",
        memmap[K230_DEV_QSPI0].base, memmap[K230_DEV_QSPI0].size);

    create_unimplemented_device("qopi1",
        memmap[K230_DEV_QSPI1].base, memmap[K230_DEV_QSPI1].size);

    create_unimplemented_device("spi",
        memmap[K230_DEV_SPI].base, memmap[K230_DEV_SPI].size);

    create_unimplemented_device("hi_sys_config",
                                memmap[K230_DEV_HI_SYS_CFG].base,
                                memmap[K230_DEV_HI_SYS_CFG].size);

    create_unimplemented_device("ddrc config",
                                memmap[K230_DEV_DDRC_CFG].base,
                                memmap[K230_DEV_DDRC_CFG].size);

    create_unimplemented_device("flash",
        memmap[K230_DEV_FLASH].base, memmap[K230_DEV_FLASH].size);
}

static const Property k230_soc_props[] = {
    DEFINE_PROP_STRING("cpu-type", K230SoCState, cpu_type),
};
static void k230_soc_class_init(ObjectClass *oc, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(oc);

    device_class_set_props(dc, k230_soc_props);
    dc->realize = k230_soc_realize;
    /* Reason: Uses serial_hds in realize function, thus can't be used twice */
    dc->user_creatable = false;
}

static const TypeInfo k230_soc_type_info = {
    .name = TYPE_RISCV_K230_SOC,
    .parent = TYPE_DEVICE,
    .instance_size = sizeof(K230SoCState),
    .instance_init = k230_soc_instance_init,
    .class_init = k230_soc_class_init,
};

static void k230_soc_register_types(void)
{
    type_register_static(&k230_soc_type_info);
}

type_init(k230_soc_register_types)
