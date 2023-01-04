/******************************************************************************
 * @file     startup_CMSDK_CM4.c
 * @brief    CMSIS Startup File for CMSDK_M4 Device
 ******************************************************************************/
/* Copyright (c) 2022 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/

#if   defined (CMSDK_CM4)    || defined (CMSDK_CM4_VHT)
  #include "CMSDK_CM4.h"
#elif defined (CMSDK_CM4_FP) || defined (CMSDK_CM4_FP_VHT)
  #include "CMSDK_CM4_FP.h"
#else
  #error device not specified!
#endif

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;

extern __NO_RETURN void __PROGRAM_START(void);

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler  (void);
            void Default_Handler(void);

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
void NMI_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void HardFault_Handler      (void) __attribute__ ((weak));
void MemManage_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler     (void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void DebugMon_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));

/* Interrupts */
void UART0RX_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void UART0TX_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void UART1RX_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void UART1TX_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void UART2RX_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void UART2TX_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0ALL_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO1ALL_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER0_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void TIMER1_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void DUALTIMER_Handler      (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI_0_1_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void UART_0_1_2_OVF_Handler (void) __attribute__ ((weak, alias("Default_Handler")));
void ETHERNET_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void I2S_Handler            (void) __attribute__ ((weak, alias("Default_Handler")));
void TOUCHSCREEN_Handler    (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO2_Handler          (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO3_Handler          (void) __attribute__ ((weak, alias("Default_Handler")));
void UART3RX_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void UART3TX_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void UART4RX_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void UART4TX_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI_2_Handler          (void) __attribute__ ((weak, alias("Default_Handler")));
void SPI_3_4_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_0_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_1_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_2_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_3_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_4_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_5_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_6_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void GPIO0_7_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
#if defined CMSDK_CM4_VHT || defined CMSDK_CM4_FP_VHT
void ARM_VSI0_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void ARM_VSI1_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void ARM_VSI2_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void ARM_VSI3_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void ARM_VSI4_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void ARM_VSI5_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void ARM_VSI6_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void ARM_VSI7_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
#endif


/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/

#if defined ( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

extern const VECTOR_TABLE_Type __VECTOR_TABLE[256];
       const VECTOR_TABLE_Type __VECTOR_TABLE[256] __VECTOR_TABLE_ATTRIBUTE = {
  (VECTOR_TABLE_Type)(&__INITIAL_SP),       /*     Initial Stack Pointer */
  Reset_Handler,                            /*     Reset Handler */
  NMI_Handler,                              /* -14 NMI Handler */
  HardFault_Handler,                        /* -13 Hard Fault Handler */
  MemManage_Handler,                        /* -12 MPU Fault Handler */
  BusFault_Handler,                         /* -11 Bus Fault Handler */
  UsageFault_Handler,                       /* -10 Usage Fault Handler */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  0,                                        /*     Reserved */
  SVC_Handler,                              /*  -5 SVC Handler */
  DebugMon_Handler,                         /*  -4 Debug Monitor Handler */
  0,                                        /*     Reserved */
  PendSV_Handler,                           /*  -2 PendSV Handler */
  SysTick_Handler,                          /*  -1 SysTick Handler */

  /* Interrupts */
  UART0RX_Handler,                          /*   0 UART 0 receive interrupt */
  UART0TX_Handler,                          /*   1 UART 0 transmit interrupt */
  UART1RX_Handler,                          /*   2 UART 1 receive interrupt */
  UART1TX_Handler,                          /*   3 UART 1 transmit interrupt */
  UART2RX_Handler,                          /*   4 UART 2 receive interrupt */
  UART2TX_Handler,                          /*   5 UART 2 transmit interrupt */
  GPIO0ALL_Handler,                         /*   6 GPIO 0 combined interrupt */
  GPIO1ALL_Handler,                         /*   7 GPIO 1 combined interrupt */
  TIMER0_Handler,                           /*   8 Timer 0 interrupt */
  TIMER1_Handler,                           /*   9 Timer 1 interrupt */
  DUALTIMER_Handler,                        /*  10 Dual Timer interrupt */
  SPI_0_1_Handler,                          /*  11 SPI 0, SPI 1 interrupt */
  UART_0_1_2_OVF_Handler,                   /*  12 UART overflow (0, 1 & 2) interrupt */
  ETHERNET_Handler,                         /*  13 Ethernet interrupt */
  I2S_Handler,                              /*  14 Audio I2S interrupt */
  TOUCHSCREEN_Handler,                      /*  15 Touch Screen interrupt */
  GPIO2_Handler,                            /*  16 GPIO 2 combined interrupt */
  GPIO3_Handler,                            /*  17 GPIO 3 combined interrupt */
  UART3RX_Handler,                          /*  18 UART 3 receive interrupt */
  UART3TX_Handler,                          /*  19 UART 3 transmit interrupt */
  UART4RX_Handler,                          /*  20 UART 4 receive interrupt */
  UART4TX_Handler,                          /*  21 UART 4 transmit interrupt */
  SPI_2_Handler,                            /*  22 SPI 2 interrupt */
  SPI_3_4_Handler,                          /*  23 SPI 3, SPI 4 interrupt */
  GPIO0_0_Handler,                          /*  24 GPIO 0 individual interrupt ( 0) */
  GPIO0_1_Handler,                          /*  25 GPIO 0 individual interrupt ( 1) */
  GPIO0_2_Handler,                          /*  26 GPIO 0 individual interrupt ( 2) */
  GPIO0_3_Handler,                          /*  27 GPIO 0 individual interrupt ( 3) */
  GPIO0_4_Handler,                          /*  28 GPIO 0 individual interrupt ( 4) */
  GPIO0_5_Handler,                          /*  29 GPIO 0 individual interrupt ( 5) */
  GPIO0_6_Handler,                          /*  30 GPIO 0 individual interrupt ( 6) */
  GPIO0_7_Handler,                          /*  31 GPIO 0 individual interrupt ( 7) */
  0,                                        /*  32 Reserved */
  0,                                        /*  33 Reserved */
  0,                                        /*  34 Reserved */
  0,                                        /*  35 Reserved */
  0,                                        /*  36 Reserved */
  0,                                        /*  37 Reserved */
  0,                                        /*  38 Reserved */
  0,                                        /*  39 Reserved */
  0,                                        /*  40 Reserved */
  0,                                        /*  41 Reserved */
  0,                                        /*  42 Reserved */
  0,                                        /*  43 Reserved */
  0,                                        /*  44 Reserved */
  0,                                        /*  45 Reserved */
  0,                                        /*  46 Reserved */
  0,                                        /*  47 Reserved */
  0,                                        /*  48 Reserved */
  0,                                        /*  49 Reserved */
  0,                                        /*  50 Reserved */
  0,                                        /*  51 Reserved */
  0,                                        /*  52 Reserved */
  0,                                        /*  53 Reserved */
  0,                                        /*  54 Reserved */
  0,                                        /*  55 Reserved */
  0,                                        /*  56 Reserved */
  0,                                        /*  57 Reserved */
  0,                                        /*  58 Reserved */
  0,                                        /*  59 Reserved */
  0,                                        /*  60 Reserved */
  0,                                        /*  61 Reserved */
  0,                                        /*  62 Reserved */
  0,                                        /*  63 Reserved */
  0,                                        /*  64 Reserved */
  0,                                        /*  65 Reserved */
  0,                                        /*  66 Reserved */
  0,                                        /*  67 Reserved */
  0,                                        /*  68 Reserved */
  0,                                        /*  69 Reserved */
  0,                                        /*  70 Reserved */
  0,                                        /*  71 Reserved */
  0,                                        /*  72 Reserved */
  0,                                        /*  73 Reserved */
  0,                                        /*  74 Reserved */
  0,                                        /*  75 Reserved */
  0,                                        /*  76 Reserved */
  0,                                        /*  77 Reserved */
  0,                                        /*  78 Reserved */
  0,                                        /*  79 Reserved */
  0,                                        /*  80 Reserved */
  0,                                        /*  81 Reserved */
  0,                                        /*  82 Reserved */
  0,                                        /*  83 Reserved */
  0,                                        /*  84 Reserved */
  0,                                        /*  85 Reserved */
  0,                                        /*  86 Reserved */
  0,                                        /*  87 Reserved */
  0,                                        /*  88 Reserved */
  0,                                        /*  89 Reserved */
  0,                                        /*  90 Reserved */
  0,                                        /*  91 Reserved */
  0,                                        /*  92 Reserved */
  0,                                        /*  93 Reserved */
  0,                                        /*  94 Reserved */
  0,                                        /*  95 Reserved */
  0,                                        /*  96 Reserved */
  0,                                        /*  97 Reserved */
  0,                                        /*  98 Reserved */
  0,                                        /*  99 Reserved */
  0,                                        /* 100 Reserved */
  0,                                        /* 101 Reserved */
  0,                                        /* 102 Reserved */
  0,                                        /* 103 Reserved */
  0,                                        /* 104 Reserved */
  0,                                        /* 105 Reserved */
  0,                                        /* 106 Reserved */
  0,                                        /* 107 Reserved */
  0,                                        /* 108 Reserved */
  0,                                        /* 109 Reserved */
  0,                                        /* 110 Reserved */
  0,                                        /* 111 Reserved */
  0,                                        /* 112 Reserved */
  0,                                        /* 113 Reserved */
  0,                                        /* 114 Reserved */
  0,                                        /* 115 Reserved */
  0,                                        /* 116 Reserved */
  0,                                        /* 117 Reserved */
  0,                                        /* 118 Reserved */
  0,                                        /* 119 Reserved */
  0,                                        /* 120 Reserved */
  0,                                        /* 121 Reserved */
  0,                                        /* 122 Reserved */
  0,                                        /* 123 Reserved */
  0,                                        /* 124 Reserved */
  0,                                        /* 125 Reserved */
  0,                                        /* 126 Reserved */
  0,                                        /* 127 Reserved */
  0,                                        /* 128 Reserved */
  0,                                        /* 129 Reserved */
  0,                                        /* 130 Reserved */
  0,                                        /* 131 Reserved */
  0,                                        /* 132 Reserved */
  0,                                        /* 133 Reserved */
  0,                                        /* 134 Reserved */
  0,                                        /* 135 Reserved */
  0,                                        /* 136 Reserved */
  0,                                        /* 137 Reserved */
  0,                                        /* 138 Reserved */
  0,                                        /* 139 Reserved */
  0,                                        /* 140 Reserved */
  0,                                        /* 141 Reserved */
  0,                                        /* 142 Reserved */
  0,                                        /* 143 Reserved */
  0,                                        /* 144 Reserved */
  0,                                        /* 145 Reserved */
  0,                                        /* 146 Reserved */
  0,                                        /* 147 Reserved */
  0,                                        /* 148 Reserved */
  0,                                        /* 149 Reserved */
  0,                                        /* 150 Reserved */
  0,                                        /* 151 Reserved */
  0,                                        /* 152 Reserved */
  0,                                        /* 153 Reserved */
  0,                                        /* 154 Reserved */
  0,                                        /* 155 Reserved */
  0,                                        /* 156 Reserved */
  0,                                        /* 157 Reserved */
  0,                                        /* 158 Reserved */
  0,                                        /* 159 Reserved */
  0,                                        /* 160 Reserved */
  0,                                        /* 161 Reserved */
  0,                                        /* 162 Reserved */
  0,                                        /* 163 Reserved */
  0,                                        /* 164 Reserved */
  0,                                        /* 165 Reserved */
  0,                                        /* 166 Reserved */
  0,                                        /* 167 Reserved */
  0,                                        /* 168 Reserved */
  0,                                        /* 169 Reserved */
  0,                                        /* 170 Reserved */
  0,                                        /* 171 Reserved */
  0,                                        /* 172 Reserved */
  0,                                        /* 173 Reserved */
  0,                                        /* 174 Reserved */
  0,                                        /* 175 Reserved */
  0,                                        /* 176 Reserved */
  0,                                        /* 177 Reserved */
  0,                                        /* 178 Reserved */
  0,                                        /* 179 Reserved */
  0,                                        /* 180 Reserved */
  0,                                        /* 181 Reserved */
  0,                                        /* 182 Reserved */
  0,                                        /* 183 Reserved */
  0,                                        /* 184 Reserved */
  0,                                        /* 185 Reserved */
  0,                                        /* 186 Reserved */
  0,                                        /* 187 Reserved */
  0,                                        /* 188 Reserved */
  0,                                        /* 189 Reserved */
  0,                                        /* 190 Reserved */
  0,                                        /* 191 Reserved */
  0,                                        /* 192 Reserved */
  0,                                        /* 193 Reserved */
  0,                                        /* 194 Reserved */
  0,                                        /* 195 Reserved */
  0,                                        /* 196 Reserved */
  0,                                        /* 197 Reserved */
  0,                                        /* 198 Reserved */
  0,                                        /* 199 Reserved */
  0,                                        /* 200 Reserved */
  0,                                        /* 201 Reserved */
  0,                                        /* 202 Reserved */
  0,                                        /* 203 Reserved */
  0,                                        /* 204 Reserved */
  0,                                        /* 205 Reserved */
  0,                                        /* 206 Reserved */
  0,                                        /* 207 Reserved */
  0,                                        /* 208 Reserved */
  0,                                        /* 209 Reserved */
  0,                                        /* 210 Reserved */
  0,                                        /* 211 Reserved */
  0,                                        /* 212 Reserved */
  0,                                        /* 213 Reserved */
  0,                                        /* 214 Reserved */
  0,                                        /* 215 Reserved */
  0,                                        /* 216 Reserved */
  0,                                        /* 217 Reserved */
  0,                                        /* 218 Reserved */
  0,                                        /* 219 Reserved */
  0,                                        /* 220 Reserved */
  0,                                        /* 221 Reserved */
  0,                                        /* 222 Reserved */
  0,                                        /* 223 Reserved */
#if defined CMSDK_CM4_VHT || defined CMSDK_CM4_FP_VHT
  ARM_VSI0_Handler,                         /* 224 VSI 0 interrupt */
  ARM_VSI1_Handler,                         /* 225 VSI 1 interrupt */
  ARM_VSI2_Handler,                         /* 226 VSI 2 interrupt */
  ARM_VSI3_Handler,                         /* 227 VSI 3 interrupt */
  ARM_VSI4_Handler,                         /* 228 VSI 4 interrupt */
  ARM_VSI5_Handler,                         /* 229 VSI 5 interrupt */
  ARM_VSI6_Handler,                         /* 230 VSI 6 interrupt */
  ARM_VSI7_Handler                          /* 231 VSI 7 interrupt */
#else
  0,                                        /* 224 Reserved */
  0,                                        /* 225 Reserved */
  0,                                        /* 226 Reserved */
  0,                                        /* 227 Reserved */
  0,                                        /* 228 Reserved */
  0,                                        /* 229 Reserved */
  0,                                        /* 230 Reserved */
  0                                         /* 231 Reserved */
#endif
};

#if defined ( __GNUC__ )
#pragma GCC diagnostic pop
#endif

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void)
{
  SystemInit();                             /* CMSIS System Initialization */
  __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}


#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

/*----------------------------------------------------------------------------
  Hard Fault Handler
 *----------------------------------------------------------------------------*/
void HardFault_Handler(void)
{
  while(1);
}

/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
  while(1);
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic pop
#endif
