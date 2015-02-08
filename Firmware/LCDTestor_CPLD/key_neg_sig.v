module key_neg_sig(
						 input clk, rstn,
						 input key_in,
						 output key_neg);
reg key_now;
reg key_pre;




always@(posedge clk or negedge rstn)
	if (!rstn)begin
		key_now <= 1'b1;
		key_pre <= 1'b1;
	end
	else begin
		key_now <= key_in;
		key_pre <= key_now;
	end
assign key_neg = (!key_now)&key_pre;


endmodule						 