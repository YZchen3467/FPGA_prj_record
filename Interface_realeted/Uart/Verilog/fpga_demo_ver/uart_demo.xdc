set_property IOSTANDARD LVDS [get_ports clk_125_p]
set_property PACKAGE_PIN F23 [get_ports clk_125_p]
set_property PACKAGE_PIN E23 [get_ports clk_125_n]
set_property IOSTANDARD LVDS [get_ports clk_125_n]

set_property PACKAGE_PIN B4 [get_ports rst_p]
set_property IOSTANDARD LVCMOS33 [get_ports rst_p]

create_clock -period 8.000 -name sys_clk [get_ports clk_125_p]

set_property PACKAGE_PIN C19 [get_ports rs232_output]
set_property IOSTANDARD LVCMOS18 [get_ports rs232_output]

set_property PACKAGE_PIN A20 [get_ports rs232_input]
set_property IOSTANDARD LVCMOS18 [get_ports rs232_input]