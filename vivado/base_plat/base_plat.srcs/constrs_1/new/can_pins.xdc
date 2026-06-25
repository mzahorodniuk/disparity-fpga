set_property PACKAGE_PIN D10 [get_ports CAN_0_0_rx] ;# PMOD1 pin 5 
set_property PACKAGE_PIN C11 [get_ports CAN_0_0_tx] ;# PMOD1 pin 7   

set_property IOSTANDARD LVCMOS33 [get_ports CAN_0_0_rx];
set_property IOSTANDARD LVCMOS33 [get_ports CAN_0_0_tx];

set_property PACKAGE_PIN K13 [get_ports CAN_1_0_rx] ;# PMOD2 pin 5 
set_property PACKAGE_PIN K12 [get_ports CAN_1_0_tx] ;# PMOD2 pin 7   

set_property IOSTANDARD LVCMOS33 [get_ports CAN_1_0_rx];
set_property IOSTANDARD LVCMOS33 [get_ports CAN_1_0_tx];