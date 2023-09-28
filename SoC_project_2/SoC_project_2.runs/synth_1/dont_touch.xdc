# This file is automatically generated.
# It contains project source information necessary for synthesis and implementation.

# Block Designs: bd/SoC/SoC.bd
set_property DONT_TOUCH TRUE [get_cells -hier -filter {REF_NAME==SoC || ORIG_REF_NAME==SoC} -quiet] -quiet

# IP: bd/SoC/ip/SoC_processing_system7_0_0/SoC_processing_system7_0_0.xci
set_property DONT_TOUCH TRUE [get_cells -hier -filter {REF_NAME==SoC_processing_system7_0_0 || ORIG_REF_NAME==SoC_processing_system7_0_0} -quiet] -quiet

# IP: bd/SoC/ip/SoC_axi_gpio_0_0/SoC_axi_gpio_0_0.xci
set_property DONT_TOUCH TRUE [get_cells -hier -filter {REF_NAME==SoC_axi_gpio_0_0 || ORIG_REF_NAME==SoC_axi_gpio_0_0} -quiet] -quiet

# IP: bd/SoC/ip/SoC_ps7_0_axi_periph_0/SoC_ps7_0_axi_periph_0.xci
set_property DONT_TOUCH TRUE [get_cells -hier -filter {REF_NAME==SoC_ps7_0_axi_periph_0 || ORIG_REF_NAME==SoC_ps7_0_axi_periph_0} -quiet] -quiet

# IP: bd/SoC/ip/SoC_rst_ps7_0_50M_0/SoC_rst_ps7_0_50M_0.xci
set_property DONT_TOUCH TRUE [get_cells -hier -filter {REF_NAME==SoC_rst_ps7_0_50M_0 || ORIG_REF_NAME==SoC_rst_ps7_0_50M_0} -quiet] -quiet

# IP: bd/SoC/ip/SoC_xbar_0/SoC_xbar_0.xci
set_property DONT_TOUCH TRUE [get_cells -hier -filter {REF_NAME==SoC_xbar_0 || ORIG_REF_NAME==SoC_xbar_0} -quiet] -quiet

# IP: bd/SoC/ip/SoC_axi_gpio_1_0/SoC_axi_gpio_1_0.xci
set_property DONT_TOUCH TRUE [get_cells -hier -filter {REF_NAME==SoC_axi_gpio_1_0 || ORIG_REF_NAME==SoC_axi_gpio_1_0} -quiet] -quiet

# IP: bd/SoC/ip/SoC_auto_pc_0/SoC_auto_pc_0.xci
set_property DONT_TOUCH TRUE [get_cells -hier -filter {REF_NAME==SoC_auto_pc_0 || ORIG_REF_NAME==SoC_auto_pc_0} -quiet] -quiet

# XDC: bd/SoC/ip/SoC_processing_system7_0_0/SoC_processing_system7_0_0.xdc
set_property DONT_TOUCH TRUE [get_cells [split [join [get_cells -hier -filter {REF_NAME==SoC_processing_system7_0_0 || ORIG_REF_NAME==SoC_processing_system7_0_0} -quiet] {/inst } ]/inst ] -quiet] -quiet

# XDC: bd/SoC/ip/SoC_axi_gpio_0_0/SoC_axi_gpio_0_0_board.xdc
set_property DONT_TOUCH TRUE [get_cells [split [join [get_cells -hier -filter {REF_NAME==SoC_axi_gpio_0_0 || ORIG_REF_NAME==SoC_axi_gpio_0_0} -quiet] {/U0 } ]/U0 ] -quiet] -quiet

# XDC: bd/SoC/ip/SoC_axi_gpio_0_0/SoC_axi_gpio_0_0_ooc.xdc

# XDC: bd/SoC/ip/SoC_axi_gpio_0_0/SoC_axi_gpio_0_0.xdc
#dup# set_property DONT_TOUCH TRUE [get_cells [split [join [get_cells -hier -filter {REF_NAME==SoC_axi_gpio_0_0 || ORIG_REF_NAME==SoC_axi_gpio_0_0} -quiet] {/U0 } ]/U0 ] -quiet] -quiet

# XDC: bd/SoC/ip/SoC_rst_ps7_0_50M_0/SoC_rst_ps7_0_50M_0_board.xdc
set_property DONT_TOUCH TRUE [get_cells [split [join [get_cells -hier -filter {REF_NAME==SoC_rst_ps7_0_50M_0 || ORIG_REF_NAME==SoC_rst_ps7_0_50M_0} -quiet] {/U0 } ]/U0 ] -quiet] -quiet

# XDC: bd/SoC/ip/SoC_rst_ps7_0_50M_0/SoC_rst_ps7_0_50M_0.xdc
#dup# set_property DONT_TOUCH TRUE [get_cells [split [join [get_cells -hier -filter {REF_NAME==SoC_rst_ps7_0_50M_0 || ORIG_REF_NAME==SoC_rst_ps7_0_50M_0} -quiet] {/U0 } ]/U0 ] -quiet] -quiet

# XDC: bd/SoC/ip/SoC_rst_ps7_0_50M_0/SoC_rst_ps7_0_50M_0_ooc.xdc

# XDC: bd/SoC/ip/SoC_xbar_0/SoC_xbar_0_ooc.xdc

# XDC: bd/SoC/ip/SoC_axi_gpio_1_0/SoC_axi_gpio_1_0_board.xdc
set_property DONT_TOUCH TRUE [get_cells [split [join [get_cells -hier -filter {REF_NAME==SoC_axi_gpio_1_0 || ORIG_REF_NAME==SoC_axi_gpio_1_0} -quiet] {/U0 } ]/U0 ] -quiet] -quiet

# XDC: bd/SoC/ip/SoC_axi_gpio_1_0/SoC_axi_gpio_1_0_ooc.xdc

# XDC: bd/SoC/ip/SoC_axi_gpio_1_0/SoC_axi_gpio_1_0.xdc
#dup# set_property DONT_TOUCH TRUE [get_cells [split [join [get_cells -hier -filter {REF_NAME==SoC_axi_gpio_1_0 || ORIG_REF_NAME==SoC_axi_gpio_1_0} -quiet] {/U0 } ]/U0 ] -quiet] -quiet

# XDC: bd/SoC/ip/SoC_auto_pc_0/SoC_auto_pc_0_ooc.xdc

# XDC: bd/SoC/SoC_ooc.xdc
