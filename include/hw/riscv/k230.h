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
#ifndef HW_K230_H
#define HW_K230_H

#include "hw/riscv/riscv_hart.h"
#include "hw/riscv/k230_cpu.h"
#include "hw/gpio/sifive_gpio.h"
/* #include "hw/misc/k230_rmu.h" */
#include "hw/boards.h"

#define TYPE_RISCV_K230_SOC "riscv.k230.soc"
#define RISCV_K230_SOC(obj) \
    OBJECT_CHECK(K230SoCState, (obj), TYPE_RISCV_K230_SOC)

typedef struct K230SoCState {
    /*< private >*/
    DeviceState parent_obj;

    /*< public >*/
    RISCVHartArrayState c908_cpus;
    RISCVHartArrayState c908v_cpus;

    DeviceState *uart[5];
    SIFIVEGPIOState gpio0;
    SIFIVEGPIOState gpio1;
    DeviceState *clint;
    DeviceState *plic;
} K230SoCState;

#define TYPE_RISCV_K230_MACHINE MACHINE_TYPE_NAME("k230")
#define RISCV_K230_MACHINE(obj) \
    OBJECT_CHECK(K230State, (obj), TYPE_RISCV_K230_MACHINE)

typedef struct K230State {
    /*< private >*/
    MachineState parent_obj;

    /*< public >*/
    K230SoCState soc;
} K230State;

enum {
    K230_DEV_DDRC,
    K230_DEV_KPU_L2_CACHE,
    K230_DEV_SRAM,
    K230_DEV_KPU_CFG,
    K230_DEV_FFT,
    K230_DEV_AI_2D_ENGINE,
    K230_DEV_GSDMA,
    K230_DEV_DMA,
    K230_DEV_DECOMP_GZIP,
    K230_DEV_NON_AI_2D,
    K230_DEV_ISP,
    K230_DEV_DEWARP,
    K230_DEV_RX_CSI,
    K230_DEV_H264,
    K230_DEV_2P5D,
    K230_DEV_VO,
    K230_DEV_VO_CFG,
    K230_DEV_3D_ENGINE,
    K230_DEV_PMU,
    K230_DEV_RTC,
    K230_DEV_CMU,
    K230_DEV_RMU,
    K230_DEV_BOOT,
    K230_DEV_PWR,
    K230_DEV_MAILBOX,
    K230_DEV_IOMUX,
    K230_DEV_TIMER,
    K230_DEV_WDT0,
    K230_DEV_WDT1,
    K230_DEV_TS,
    K230_DEV_HDI,
    K230_DEV_STC,
    K230_DEV_BOOTROM,
    K230_DEV_SECURITY,
    K230_DEV_UART0,
    K230_DEV_UART1,
    K230_DEV_UART2,
    K230_DEV_UART3,
    K230_DEV_UART4,
    K230_DEV_I2C0,
    K230_DEV_I2C1,
    K230_DEV_I2C2,
    K230_DEV_I2C3,
    K230_DEV_I2C4,
    K230_DEV_PWM,
    K230_DEV_GPIO0,
    K230_DEV_GPIO1,
    K230_DEV_ADC,
    K230_DEV_CODEC,
    K230_DEV_AUDIO,
    K230_DEV_USB,
    K230_DEV_SD,
    K230_DEV_SPI_QOPI,
    K230_DEV_SPI_OPI,
    K230_DEV_HI_SYS_CONFIG,
    K230_DEV_DDRC_CONFIG,
    K230_DEV_FLASH,
    K230_DEV_CLINT, /* fixme */
    K230_DEV_PLIC,
};

enum {
    K230_UART0_IRQ  = 0,
    K230_UART1_IRQ  = 1,
    K230_UART2_IRQ  = 2,
    K230_UART3_IRQ  = 3,
    K230_UART4_IRQ  = 4,
    K230_PWM0_IRQ   = 10,
    K230_PWM1_IRQ   = 11,
    K230_PWM2_IRQ   = 12,
    K230_PWM3_IRQ   = 13,
    K230_PWM4_IRQ   = 14,
    K230_PWM5_IRQ   = 15,
    K230_GPIO0_IRQ0 = 16
};

#define K230_UART_IRQS 5
#define K230_GPIO_IRQS 64

#define K230_LFCLK_DEFAULT_FREQ 24000000  /* fixme */

#define K230_PLIC_HART_CONFIG "MS,MS"
#define K230_PLIC_NUM_SOURCES 208
#define K230_PLIC_NUM_PRIORITIES 7  /* fixme */

#define K230_PLIC_PRIORITY_BASE 0x00
#define K230_PLIC_PENDING_BASE 0x1000
#define K230_PLIC_ENABLE_BASE 0x2000
#define K230_PLIC_ENABLE_STRIDE 0x80

#define K230_PLIC_CONTEXT_BASE 0x200000
#define K230_PLIC_CONTEXT_STRIDE 0x1000

#endif
