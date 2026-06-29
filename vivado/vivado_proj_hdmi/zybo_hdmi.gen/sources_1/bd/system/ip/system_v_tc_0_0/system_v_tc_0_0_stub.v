// Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
// Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2025.2 (win64) Build 6299465 Fri Nov 14 19:35:11 GMT 2025
// Date        : Mon Jun 29 10:58:32 2026
// Host        : LAPTOP-3SK89RUD running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub
//               c:/Users/user/Documents/AAD/space_invaders/vivado/vivado_proj_hdmi/zybo_hdmi.gen/sources_1/bd/system/ip/system_v_tc_0_0/system_v_tc_0_0_stub.v
// Design      : system_v_tc_0_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7z020clg400-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* CHECK_LICENSE_TYPE = "system_v_tc_0_0,v_tc,{}" *) (* core_generation_info = "system_v_tc_0_0,v_tc,{x_ipProduct=Vivado 2025.2,x_ipVendor=xilinx.com,x_ipLibrary=ip,x_ipName=v_tc,x_ipVersion=6.2,x_ipCoreRevision=11,x_ipLanguage=VERILOG,x_ipSimLanguage=MIXED,C_HAS_AXI4_LITE=0,C_HAS_INTC_IF=0,C_ARBITRARY_RES_EN=0,C_VID_PPC=4,C_GEN_INTERLACED=0,C_GEN_HACTIVE_SIZE=640,C_GEN_VACTIVE_SIZE=480,C_GEN_CPARITY=0,C_GEN_FIELDID_POLARITY=1,C_GEN_VBLANK_POLARITY=1,C_GEN_HBLANK_POLARITY=1,C_GEN_VSYNC_POLARITY=0,C_GEN_HSYNC_POLARITY=0,C_GEN_AVIDEO_POLARITY=1,C_GEN_ACHROMA_POLARITY=1,C_GEN_VIDEO_FORMAT=2,C_GEN_HFRAME_SIZE=800,C_GEN_F0_VFRAME_SIZE=525,C_GEN_F1_VFRAME_SIZE=750,C_GEN_HSYNC_START=656,C_GEN_HSYNC_END=752,C_GEN_F0_VBLANK_HSTART=640,C_GEN_F0_VBLANK_HEND=640,C_GEN_F0_VSYNC_VSTART=490,C_GEN_F0_VSYNC_VEND=492,C_GEN_F0_VSYNC_HSTART=656,C_GEN_F0_VSYNC_HEND=656,C_GEN_F1_VBLANK_HSTART=640,C_GEN_F1_VBLANK_HEND=640,C_GEN_F1_VSYNC_VSTART=724,C_GEN_F1_VSYNC_VEND=729,C_GEN_F1_VSYNC_HSTART=695,C_GEN_F1_VSYNC_HEND=695,C_FSYNC_HSTART0=0,C_FSYNC_VSTART0=0,C_FSYNC_HSTART1=0,C_FSYNC_VSTART1=0,C_FSYNC_HSTART2=0,C_FSYNC_VSTART2=0,C_FSYNC_HSTART3=0,C_FSYNC_VSTART3=0,C_FSYNC_HSTART4=0,C_FSYNC_VSTART4=0,C_FSYNC_HSTART5=0,C_FSYNC_VSTART5=0,C_FSYNC_HSTART6=0,C_FSYNC_VSTART6=0,C_FSYNC_HSTART7=0,C_FSYNC_VSTART7=0,C_FSYNC_HSTART8=0,C_FSYNC_VSTART8=0,C_FSYNC_HSTART9=0,C_FSYNC_VSTART9=0,C_FSYNC_HSTART10=0,C_FSYNC_VSTART10=0,C_FSYNC_HSTART11=0,C_FSYNC_VSTART11=0,C_FSYNC_HSTART12=0,C_FSYNC_VSTART12=0,C_FSYNC_HSTART13=0,C_FSYNC_VSTART13=0,C_FSYNC_HSTART14=0,C_FSYNC_VSTART14=0,C_FSYNC_HSTART15=0,C_FSYNC_VSTART15=0,C_MAX_PIXELS=4096,C_MAX_LINES=4096,C_NUM_FSYNCS=1,C_INTERLACE_EN=0,C_GEN_AUTO_SWITCH=0,C_DETECT_EN=0,C_SYNC_EN=0,C_GENERATE_EN=1,C_DET_HSYNC_EN=1,C_DET_VSYNC_EN=1,C_DET_HBLANK_EN=1,C_DET_VBLANK_EN=1,C_DET_AVIDEO_EN=1,C_DET_ACHROMA_EN=0,C_GEN_HSYNC_EN=1,C_GEN_VSYNC_EN=1,C_GEN_HBLANK_EN=1,C_GEN_VBLANK_EN=1,C_GEN_AVIDEO_EN=1,C_GEN_ACHROMA_EN=0,C_GEN_FIELDID_EN=0,C_DET_FIELDID_EN=0}" *) (* downgradeipidentifiedwarnings = "yes" *) 
(* x_core_info = "v_tc,Vivado 2025.2" *) 
module system_v_tc_0_0(clk, clken, gen_clken, sof_state, hsync_out, 
  hblank_out, vsync_out, vblank_out, active_video_out, resetn, fsync_out)
/* synthesis syn_black_box black_box_pad_pin="clken,gen_clken,sof_state,hsync_out,hblank_out,vsync_out,vblank_out,active_video_out,resetn,fsync_out[0:0]" */
/* synthesis syn_force_seq_prim="clk" */;
  (* x_interface_info = "xilinx.com:signal:clock:1.0 clk_intf CLK" *) (* x_interface_mode = "slave clk_intf" *) (* x_interface_parameter = "XIL_INTERFACENAME clk_intf, ASSOCIATED_BUSIF vtiming_in:vtiming_out, ASSOCIATED_RESET resetn, ASSOCIATED_CLKEN clken, FREQ_HZ 25178571, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN /clk_wiz_0_clk_out1, INSERT_VIP 0" *) input clk /* synthesis syn_isclock = 1 */;
  (* x_interface_info = "xilinx.com:signal:clockenable:1.0 clken_intf CE" *) (* x_interface_mode = "slave clken_intf" *) (* x_interface_parameter = "XIL_INTERFACENAME clken_intf, POLARITY ACTIVE_HIGH" *) input clken;
  input gen_clken;
  input sof_state;
  (* x_interface_info = "xilinx.com:interface:video_timing:2.0 vtiming_out HSYNC" *) (* x_interface_mode = "master vtiming_out" *) output hsync_out;
  (* x_interface_info = "xilinx.com:interface:video_timing:2.0 vtiming_out HBLANK" *) output hblank_out;
  (* x_interface_info = "xilinx.com:interface:video_timing:2.0 vtiming_out VSYNC" *) output vsync_out;
  (* x_interface_info = "xilinx.com:interface:video_timing:2.0 vtiming_out VBLANK" *) output vblank_out;
  (* x_interface_info = "xilinx.com:interface:video_timing:2.0 vtiming_out ACTIVE_VIDEO" *) output active_video_out;
  (* x_interface_info = "xilinx.com:signal:reset:1.0 resetn_intf RST" *) (* x_interface_mode = "slave resetn_intf" *) (* x_interface_parameter = "XIL_INTERFACENAME resetn_intf, POLARITY ACTIVE_LOW, INSERT_VIP 0" *) input resetn;
  output [0:0]fsync_out;
endmodule
