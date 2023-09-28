onbreak {quit -f}
onerror {quit -f}

vsim -t 1ps -lib xil_defaultlib SoC_opt

do {wave.do}

view wave
view structure
view signals

do {SoC.udo}

run -all

quit -force
