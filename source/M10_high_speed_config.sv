/*
###############################################################################
# Copyright (c) 2017, PulseRain Technology LLC 
#
# This program is distributed under a dual license: an open source license, 
# and a commercial license. 
# 
# The open source license under which this program is distributed is the 
# GNU Public License version 3 (GPLv3).
#
# And for those who want to use this program in ways that are incompatible
# with the GPLv3, PulseRain Technology LLC offers commercial license instead.
# Please contact PulseRain Technology LLC (www.pulserain.com) for more detail.
#
###############################################################################
 */


//=============================================================================
// Remarks:
//     Top Level Module for PulseRain M10 board
//=============================================================================


`include "debug_coprocessor.svh"
`include "common.svh"


`default_nettype none

module M10_high_speed_config (
        
    //=======================================================================
    // clock / reset
    //=======================================================================
        input wire                                  osc_in,         // expecting 12MHz oscillator in
        input wire                                  push_button,    // push button for reset
        
    //=======================================================================
    // external interrupt
    //=======================================================================
        
       // input wire  unsigned [1 : 0]                INTx,
    
    //=======================================================================
    // IO port
    //=======================================================================
        
            
        inout wire unsigned [7 : 0]                 P0,
        inout wire unsigned [7 : 0]                 P1,
     //   inout wire unsigned [7 : 0] P2,
     //   inout wire unsigned [7 : 0] P3,
        
    //=======================================================================
    // UART
    //=======================================================================
        input wire                                  UART_RXD,
        output logic                                UART_TXD,
        
    //=======================================================================
    // LED
    //=======================================================================
        output wire                                 debug_led,
        output wire											 red_led,
        output wire                                 green_led		  
   
);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Signals
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        wire                                                              clk, clk_24MHz, clk_2MHz;
        logic                                                             classic_12MHz_pulse;
        wire                                                              pll_locked;
    
        wire                                                              pram_read_enable_in;
        wire unsigned [DEBUG_DATA_WIDTH * DEBUG_FRAME_DATA_LEN - 1 : 0]   pram_read_data_in;
    
        wire                                                              pram_read_enable_out;
        wire unsigned [DEBUG_PRAM_ADDR_WIDTH - 1 : 0]                     pram_read_addr_out;
        wire                                                              pram_write_enable_out;
        wire unsigned [DEBUG_PRAM_ADDR_WIDTH - 3 : 0]                     pram_write_addr_out;
        wire unsigned [DEBUG_DATA_WIDTH * DEBUG_FRAME_DATA_LEN - 1 : 0]   pram_write_data_out;
        
        wire                                                              debug_stall_flag;
        wire unsigned [PC_BITWIDTH - 1 : 0]                               PC;
    
        wire                                                              cpu_reset;
        wire                                                              pause;
        wire                                                              break_on;
        wire unsigned [PC_BITWIDTH - 1 : 0]                               break_addr_A;
        wire unsigned [PC_BITWIDTH - 1 : 0]                               break_addr_B;
    
        wire                                                              run_pulse;
    
        wire                                                              debug_read_data_enable;
        wire unsigned [DEBUG_DATA_WIDTH - 1 : 0]                          debug_read_data;
    
        wire                                                              debug_data_read;
        wire unsigned [DEBUG_PRAM_ADDR_WIDTH - 1 : 0]                     debug_data_read_addr;
        wire                                                              debug_data_read_restore;
        wire unsigned [15 : 0]                                            debug_read_data_out;         
        wire                                                              debug_rd_indirect1_direct0;
        wire                                                              debug_counter_pulse;
            
        wire                                                              timer_pulse;    
              
        wire                                                              debug_data_write;
        wire  unsigned [DEBUG_PRAM_ADDR_WIDTH - 1 : 0]                    debug_data_write_addr;
        wire                                                              debug_wr_indirect1_direct0;
        wire  unsigned [DEBUG_DATA_WIDTH - 1 : 0]                         debug_data_write_data;
        
        wire                                                              debug_uart_tx_sel_ocd1_cpu0;
        wire                                                              uart_tx_cpu;
        wire                                                              uart_tx_ocd;
    
        
        wire                                                              reset_button;
        
        wire                                                              debug_led_i;
           
        wire unsigned [7 : 0]                                             P2;
        wire unsigned [7 : 0]                                             P3;
  
        wire                                                              flash_loader_active_flag;
        wire                                                              flash_loader_done_flag;
        
        wire                                                              flash_loader_ping_busy;
        wire                                                              flash_loader_pong_busy;
   
        wire  unsigned [1 : 0]                                            flash_buffer_ping_state;
        wire  unsigned [1 : 0]                                            flash_buffer_pong_state;
        
		  wire  unsigned [1 : 0]                									  INTx;
		  
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // PLL and clock control block
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            
        PLL PLL_i (
            .areset (reset_button),
            .inclk0 (osc_in),
            .c0 (clk),
            .c1 (clk_24MHz),
            .c2 (clk_2MHz),
            .locked (pll_locked));
		            
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // button debouncer
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        switch_debouncer   #(.TIMER_VALUE(100000)) switch_debouncer_i (
            .clk (osc_in),
            .reset_n (1'b1),  
            .data_in (~push_button),
            .data_out (reset_button)
        );
    
		  
	 
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // No Intrrrupt
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    	  
		  
		  assign INTx = 0;
		  
	  
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // UART
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        
        always_ff @(posedge  clk, negedge pll_locked) begin
            if (!pll_locked) begin
                UART_TXD <= 0;
            end else if (!debug_uart_tx_sel_ocd1_cpu0) begin
                UART_TXD <= uart_tx_cpu;
            end else begin
                UART_TXD <= uart_tx_ocd;
            end
        
        end 
    

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  onchip debugger
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        
        debug_coprocessor_wrapper #(.BAUD_PERIOD (MIN_UART_BAID_PERIOD)) 
            debug_coprocessor_wrapper_i (
                .clk (clk),
                .reset_n (pll_locked),
                
                .RXD (UART_RXD),
                .TXD (uart_tx_ocd),
                
                .debug_counter_pulse  (1'b0),
                .timer_pulse (1'b0),
            
                .debug_stall_flag (debug_stall_flag),
               //== .PC (PC),
                // use PC to pass flash_loader status
                .PC ({flash_buffer_ping_state, flash_buffer_pong_state, flash_loader_active_flag, flash_loader_done_flag, flash_loader_ping_busy, flash_loader_pong_busy}), 
                   
                
                .pram_read_enable_in (pram_read_enable_in),
                .pram_read_data_in (pram_read_data_in),
        
                .pram_read_enable_out (pram_read_enable_out),
                .pram_write_enable_out (pram_write_enable_out),
                .pram_write_addr_out (pram_write_addr_out),
                .pram_read_addr_out (pram_read_addr_out),
                    
                .pram_write_data_out (pram_write_data_out),
                
                .cpu_reset (cpu_reset),
                .pause (pause),
                .break_on (break_on),
                .break_addr_A (break_addr_A),
                .break_addr_B (break_addr_B),
                .run_pulse (run_pulse),
                
                .debug_read_data_enable_in (debug_read_data_enable),
                .debug_read_data_in (debug_read_data),
                
                .debug_data_read (debug_data_read),
                .debug_data_read_addr (debug_data_read_addr),
                .debug_rd_indirect1_direct0 (debug_rd_indirect1_direct0),
                .debug_data_read_restore (debug_data_read_restore),
                
                .debug_data_write (debug_data_write),
                .debug_data_write_addr (debug_data_write_addr),
                .debug_wr_indirect1_direct0 (debug_wr_indirect1_direct0),
                .debug_data_write_data (debug_data_write_data),
                .debug_uart_tx_sel_ocd1_cpu0 (debug_uart_tx_sel_ocd1_cpu0)
                
            );
            
    
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  M10_config_MCU
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            
         PulseRain_M10_config_MCU #(.FOR_SIM (0), .FAST0_SMALL1 (0)) M10_config_MCU (
                .clk (clk),
                .reset_n (pll_locked & (~cpu_reset)),
                
                .INTx (INTx),
                .inst_mem_we      (pram_write_enable_out),
                .inst_mem_wr_addr (pram_write_addr_out),
                .inst_mem_data_in (pram_write_data_out),
                
                .inst_mem_re (pram_read_enable_out),
                .inst_mem_re_addr (pram_read_addr_out),
                
                .inst_mem_re_enable_out (pram_read_enable_in),
                .inst_mem_data_out (pram_read_data_in),
                            
                .UART_RXD (UART_RXD),
                .UART_TXD (uart_tx_cpu),
                
                .P0 (P0),
                .P1 (P1),
                .P2 (P2),
                .P3 (P3),
                 
                 
                .pause (pause),
                .break_on (break_on),
                .break_addr_A (break_addr_A),
                .break_addr_B (break_addr_B),
                .run_pulse (run_pulse),
        
                .debug_stall (debug_stall_flag),
                .debug_PC (PC),
            
                .debug_data_read (debug_data_read),
                .debug_data_read_addr (debug_data_read_addr),
                .debug_rd_indirect1_direct0 (debug_rd_indirect1_direct0),
                .debug_data_read_restore (debug_data_read_restore),
            
            
                .debug_data_write (debug_data_write),
                .debug_data_write_addr (debug_data_write_addr),
                .debug_wr_indirect1_direct0 (debug_wr_indirect1_direct0),
                .debug_data_write_data (debug_data_write_data),    
            
            
                .debug_read_data_enable_out (debug_read_data_enable),
                .debug_read_data_out (debug_read_data),
                .timer_pulse_out (timer_pulse),
                 
                .debug_led (debug_led_i),
                .debug_counter_pulse (debug_counter_pulse),
                
                .flash_loader_active_flag (flash_loader_active_flag),
                .flash_loader_done_flag (flash_loader_done_flag),
                .flash_loader_ping_busy (flash_loader_ping_busy),
                .flash_loader_pong_busy (flash_loader_pong_busy),
                .flash_buffer_ping_state (flash_buffer_ping_state),
                .flash_buffer_pong_state (flash_buffer_pong_state)
        );    
     
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  led
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        assign debug_led = (debug_led_i) | (~push_button);
    
	     assign red_led    = 1'b1;
		  assign green_led  = 1'b0;
		  
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  dual config
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        
    
        dual_config dual_config_i (
            .avmm_rcv_address (3'd0),   // avalon.address
            .avmm_rcv_read (1'b0),      //       .read
            .avmm_rcv_writedata (32'd0), //       .writedata
            .avmm_rcv_write (1'b0),     //       .write
            .avmm_rcv_readdata(),  //       .readdata
            .clk (clk_24MHz),                //    clk.clk
            .nreset (1'b1)              // nreset.reset_n
        );
    

endmodule : M10_high_speed_config


`default_nettype wire
