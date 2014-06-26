# TCL File Generated by Component Editor 13.1
# Wed May 14 09:47:33 CEST 2014
# DO NOT MODIFY


# 
# usb_ft232 "usb_ft232" v1.0
# Beat Meier 2014.05.14.09:47:33
# ft232 interface with streaming output
# 

# 
# request TCL package from ACDS 13.1
# 
package require -exact qsys 13.1


# 
# module usb_ft232
# 
set_module_property DESCRIPTION "ft232 interface with streaming output"
set_module_property NAME usb_ft232
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property GROUP DTB
set_module_property AUTHOR "Beat Meier"
set_module_property DISPLAY_NAME usb_ft232
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property ANALYZE_HDL AUTO
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false


# 
# file sets
# 
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL usb16
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
add_fileset_file usb16.v VERILOG PATH ip/usb_ft232/usb16.v TOP_LEVEL_FILE
add_fileset_file usb_avalon16.v VERILOG PATH ip/usb_ft232/usb_avalon16.v
add_fileset_file usb_ft232.v VERILOG PATH ip/usb_ft232/usb_ft232.v
add_fileset_file usb_rx_fifo.v VERILOG PATH ip/usb_ft232/usb_rx_fifo.v
add_fileset_file usb_tx_fifo.v VERILOG PATH ip/usb_ft232/usb_tx_fifo.v


# 
# parameters
# 


# 
# display items
# 


# 
# connection point clock
# 
add_interface clock clock end
set_interface_property clock clockRate 0
set_interface_property clock ENABLED true
set_interface_property clock EXPORT_OF ""
set_interface_property clock PORT_NAME_MAP ""
set_interface_property clock CMSIS_SVD_VARIABLES ""
set_interface_property clock SVD_ADDRESS_GROUP ""

add_interface_port clock clk clk Input 1


# 
# connection point reset
# 
add_interface reset reset end
set_interface_property reset associatedClock clock
set_interface_property reset synchronousEdges DEASSERT
set_interface_property reset ENABLED true
set_interface_property reset EXPORT_OF ""
set_interface_property reset PORT_NAME_MAP ""
set_interface_property reset CMSIS_SVD_VARIABLES ""
set_interface_property reset SVD_ADDRESS_GROUP ""

add_interface_port reset reset reset Input 1


# 
# connection point ctrl
# 
add_interface ctrl avalon end
set_interface_property ctrl addressUnits WORDS
set_interface_property ctrl associatedClock clock
set_interface_property ctrl associatedReset reset
set_interface_property ctrl bitsPerSymbol 8
set_interface_property ctrl burstOnBurstBoundariesOnly false
set_interface_property ctrl burstcountUnits WORDS
set_interface_property ctrl explicitAddressSpan 0
set_interface_property ctrl holdTime 0
set_interface_property ctrl linewrapBursts false
set_interface_property ctrl maximumPendingReadTransactions 0
set_interface_property ctrl readLatency 0
set_interface_property ctrl readWaitStates 0
set_interface_property ctrl readWaitTime 0
set_interface_property ctrl setupTime 0
set_interface_property ctrl timingUnits Cycles
set_interface_property ctrl writeWaitTime 0
set_interface_property ctrl ENABLED true
set_interface_property ctrl EXPORT_OF ""
set_interface_property ctrl PORT_NAME_MAP ""
set_interface_property ctrl CMSIS_SVD_VARIABLES ""
set_interface_property ctrl SVD_ADDRESS_GROUP ""

add_interface_port ctrl avs_ctrl_address address Input 1
add_interface_port ctrl avs_ctrl_write write Input 1
add_interface_port ctrl avs_ctrl_writedata writedata Input 8
add_interface_port ctrl avs_ctrl_read read Input 1
add_interface_port ctrl avs_ctrl_readdata readdata Output 8
set_interface_assignment ctrl embeddedsw.configuration.isFlash 0
set_interface_assignment ctrl embeddedsw.configuration.isMemoryDevice 0
set_interface_assignment ctrl embeddedsw.configuration.isNonVolatileStorage 0
set_interface_assignment ctrl embeddedsw.configuration.isPrintableDevice 0


# 
# connection point uplink
# 
add_interface uplink avalon_streaming end
set_interface_property uplink associatedClock clock
set_interface_property uplink associatedReset reset
set_interface_property uplink dataBitsPerSymbol 8
set_interface_property uplink errorDescriptor ""
set_interface_property uplink firstSymbolInHighOrderBits true
set_interface_property uplink maxChannel 0
set_interface_property uplink readyLatency 0
set_interface_property uplink ENABLED true
set_interface_property uplink EXPORT_OF ""
set_interface_property uplink PORT_NAME_MAP ""
set_interface_property uplink CMSIS_SVD_VARIABLES ""
set_interface_property uplink SVD_ADDRESS_GROUP ""

add_interface_port uplink asi_uplink_ready ready Output 1
add_interface_port uplink asi_uplink_valid valid Input 1
add_interface_port uplink asi_uplink_data data Input 16
add_interface_port uplink asi_uplink_startofpacket startofpacket Input 1
add_interface_port uplink asi_uplink_endofpacket endofpacket Input 1
add_interface_port uplink asi_uplink_empty empty Input 1


# 
# connection point ft232h
# 
add_interface ft232h conduit end
set_interface_property ft232h associatedClock clock
set_interface_property ft232h associatedReset ""
set_interface_property ft232h ENABLED true
set_interface_property ft232h EXPORT_OF ""
set_interface_property ft232h PORT_NAME_MAP ""
set_interface_property ft232h CMSIS_SVD_VARIABLES ""
set_interface_property ft232h SVD_ADDRESS_GROUP ""

add_interface_port ft232h usb_clk export Input 1
add_interface_port ft232h wr_n export Output 1
add_interface_port ft232h rd_n export Output 1
add_interface_port ft232h oe_n export Output 1
add_interface_port ft232h data export Bidir 8
add_interface_port ft232h txe_n export Input 1
add_interface_port ft232h rxf_n export Input 1
add_interface_port ft232h siwu_n export Output 1
add_interface_port ft232h db_tx_full export Output 1
add_interface_port ft232h db_tx_empty export Output 1
add_interface_port ft232h db_tx_write export Output 1
add_interface_port ft232h db_tx_read export Output 1

