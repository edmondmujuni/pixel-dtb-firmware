# deser400.sdc


# --- phase_detector
set_false_path -from {*|phase_detector:inst31|inst1[*]} -to {*}



# --- frame_detector
set_false_path -from {*|frame_detector:inst30|dffom:inst377|inst} -to {*}


# --- NRZI_5B4B_decoder
set_multicycle_path \
	-from {*|NRZI_5B4B_decoder:inst4|lpm_dff0:inst487|lpm_ff:lpm_ff_component|dffs[*]} \
	-setup 4
set_multicycle_path \
	-from {*|NRZI_5B4B_decoder:inst4|lpm_dff0:inst487|lpm_ff:lpm_ff_component|dffs[*]} \
	-hold 3
	
set_multicycle_path \
	-from {*|NRZI_5B4B_decoder:inst4|inst4487} \
	-setup 4
set_multicycle_path \
	-from {*|NRZI_5B4B_decoder:inst4|inst4487} \
	-hold 3

set_multicycle_path \
	-from {*|NRZI_5B4B_decoder:inst4|NRZI_decoder:inst4003|OUT_5b_o[*]} \
	-to   {*|NRZI_5B4B_decoder:inst4|NRZI_decoder:inst4003|OUT_5b_o[*]} \
	-setup 3
set_multicycle_path \
	-from {*|NRZI_5B4B_decoder:inst4|NRZI_decoder:inst4003|OUT_5b_o[*]} \
	-to   {*|NRZI_5B4B_decoder:inst4|NRZI_decoder:inst4003|OUT_5b_o[*]} \
	-hold 2

set_multicycle_path \
	-from {*|NRZI_5B4B_decoder:inst4|DECODE_5b4b:inst488|OUT_4b_o[*]} \
	-to {*|demux_tbmAB:inst7|gl_dff4e:inst469|lpm_ff:lpm_ff_component|dffs[*]} \
	-setup 3
set_multicycle_path \
	-from {*|NRZI_5B4B_decoder:inst4|DECODE_5b4b:inst488|OUT_4b_o[*]} \
	-to {*|demux_tbmAB:inst7|gl_dff4e:inst469|lpm_ff:lpm_ff_component|dffs[*]} \
	-hold 2

set_false_path -from {*|NRZI_5B4B_decoder:inst4|NRZI_decoder:inst4003|error} -to {*}