module get_key(
						 input clk, rstn,
						 input key_in,
						 input key_neg,
						 output reg key_out);

parameter T_10ms = 22'd1250_000; //10Ms,125M/100

reg[31:0]cnt_T;

always@(posedge clk or negedge rstn)
	if (!rstn)
		cnt_T <= 0;
	else if (key_neg)
		cnt_T <= 0;
	else
		cnt_T <= cnt_T +1'b1;

always@(posedge clk or negedge rstn)
	if (!rstn)
		key_out <= 1'b0;
	else if (cnt_T == T_10ms)
		key_out <= ~key_in;
	else
		key_out <= 1'b0;
		
endmodule
		