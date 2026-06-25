//Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
//--------------------------------------------------------------------------------
//Tool Version: Vivado v.2022.2 (lin64) Build 3671981 Fri Oct 14 04:59:54 MDT 2022
//Date        : Wed May  7 15:08:39 2025
//Host        : kria running 64-bit Ubuntu 22.04.1 LTS
//Command     : generate_target top_wrapper.bd
//Design      : top_wrapper
//Purpose     : IP block netlist
//--------------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module top_wrapper
   (CAN_0_0_rx,
    CAN_0_0_tx,
    CAN_1_0_rx,
    CAN_1_0_tx);
  input CAN_0_0_rx;
  output CAN_0_0_tx;
  input CAN_1_0_rx;
  output CAN_1_0_tx;

  wire CAN_0_0_rx;
  wire CAN_0_0_tx;
  wire CAN_1_0_rx;
  wire CAN_1_0_tx;

  top top_i
       (.CAN_0_0_rx(CAN_0_0_rx),
        .CAN_0_0_tx(CAN_0_0_tx),
        .CAN_1_0_rx(CAN_1_0_rx),
        .CAN_1_0_tx(CAN_1_0_tx));
endmodule
