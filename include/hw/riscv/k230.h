/*
 * QEMU RISC-V Virt Board Compatible with kendryte K230 SDK
 *
 * Copyright (c) 2025 Chao Liu <chao.liu@zevorn.cn>
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
    DeviceState *gpio;
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
    K230_DEV_USB,
    K230_DEV_LSADC,
    K230_DEV_SPI,
    K230_DEV_SD,
    K230_DEV_CLINT,
    K230_DEV_PLIC,
    K230_DEV_DRAM,
};

#endif
