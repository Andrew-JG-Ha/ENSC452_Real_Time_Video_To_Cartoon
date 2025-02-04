-- Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2020.2 (win64) Build 3064766 Wed Nov 18 09:12:45 MST 2020
-- Date        : Thu Apr  4 23:34:59 2024
-- Host        : Shonk running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode synth_stub
--               s:/Downloads/Testing/Real_Time_Video_Filter/Real_Time_Video_Filter.gen/sources_1/bd/Real_Time_Video_Filter/ip/Real_Time_Video_Filter_ov7670_capture_0_0/Real_Time_Video_Filter_ov7670_capture_0_0_stub.vhdl
-- Design      : Real_Time_Video_Filter_ov7670_capture_0_0
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xc7z020clg484-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity Real_Time_Video_Filter_ov7670_capture_0_0 is
  Port ( 
    pclk : in STD_LOGIC;
    vsync : in STD_LOGIC;
    href : in STD_LOGIC;
    d : in STD_LOGIC_VECTOR ( 7 downto 0 );
    addr : out STD_LOGIC_VECTOR ( 18 downto 0 );
    dout : out STD_LOGIC_VECTOR ( 11 downto 0 );
    we : out STD_LOGIC;
    aclk : out STD_LOGIC
  );

end Real_Time_Video_Filter_ov7670_capture_0_0;

architecture stub of Real_Time_Video_Filter_ov7670_capture_0_0 is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "pclk,vsync,href,d[7:0],addr[18:0],dout[11:0],we,aclk";
attribute x_core_info : string;
attribute x_core_info of stub : architecture is "ov7670_capture,Vivado 2020.2";
begin
end;
