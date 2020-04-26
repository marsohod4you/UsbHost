
module switcher(
	input wire sel,
	input wire rd,
	input wire wr,
	output wire rd0,
	output wire rd1,
	output wire wr0,
	output wire wr1,
	input wire drdy0,
	input wire drdy1,
	input wire [7:0]d0,
	input wire [7:0]d1,
	output wire drdy,
	output wire [7:0]d
);

assign rd0 = (!sel) & rd;
assign wr0 = (!sel) & wr;
assign rd1 = ( sel) & rd;
assign wr1 = ( sel) & wr;

assign drdy = sel ? drdy1 : drdy0;
assign d    = sel ? d1 : d0;

endmodule
