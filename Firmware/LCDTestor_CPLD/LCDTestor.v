
//本例是通过一个二进制计数器对时钟进行计数分频，来控制LED闪烁
//计数器每个时钟加一,观察一个3位的二进制计数器，它依次变化是
// 000 001 010 011 100 101 110 111 000 001...
// 如果我们将它的D3位接到led上，那它的输出频率就是输入的8分频
// 通过不同的分频，我们可以控制led的闪烁速度。
module LCDTestor
(
	clk,rst_n,key_in,
	vga_r,vga_g,vga_b,
	vga_hsy,vga_vsy,vga_clk,vga_de,
	led,led1
);

parameter WIDTH = 32;
input	clk;
input rst_n, key_in; 
output[7:0] vga_r;
output[7:0] vga_g;
output[7:0] vga_b;
output reg vga_hsy,vga_vsy;
output reg vga_clk, vga_de;
output reg led;
wire key_out;
output led1;
reg [WIDTH-1:0] count = 32'd0;

`define VGA_1024_600
	// Reset if needed, or increment if counting is enabled


`ifdef VGA_1024_600
	//VGA Timing 1024*768 & 65MHz & 60Hz

	parameter VGA_HTT = 12'd1344-12'd1;	//Hor Total Time
	parameter VGA_HST = 12'd136;		//Hor Sync  Time
	parameter VGA_HBP = 12'd160;		//Hor Back Porch
	parameter VGA_HVT = 12'd1024;	//Hor Valid Time
	parameter VGA_HFP = 12'd24;		//Hor Front Porch

	parameter VGA_VTT = 12'd806-12'd1;	//Ver Total Time
	parameter VGA_VST = 12'd6;		//Ver Sync Time
	parameter VGA_VBP = 12'd29;		//Ver Back Porch
	parameter VGA_VVT = 12'd600;	//Ver Valid Time
	parameter VGA_VFP = 12'd3;		//Ver Front Porch
		
	parameter VGA_CORBER = 12'd128;	//8�ȷ���Color bar��ʾ
`endif

//-----------------------------------------------------------
//调用KEY检测

//-----------------------------------------------------------
	//x and y counter
reg[11:0] xcnt;
reg[11:0] ycnt;
reg		 dvi_clk;
reg vga_valid;
reg key_now;
reg key_pre;

wire key_neg;

key_neg_sig U1(
					.clk(clk),
					.rstn(rst_n),
					.key_in(key_in),
					.key_neg(key_neg));
get_key U2(
				.clk(clk),
				.rstn(rst_n),
				.key_in(key_in),
				.key_neg(key_neg),
				.key_out(key_out));
				
/*always@(posedge clk or negedge rst_n)
	if (!rst_n)begin
		key_now <= 1'b1;
		key_pre <= 1'b1;
	end
	else begin
		key_now <= key_in;
		key_pre <= key_now;
	end*/
	
always @(posedge clk or negedge rst_n)
	if(!rst_n)begin
		dvi_clk <= 1'b0;
		vga_clk <= 1'b0;
	end
	else begin
		count <= count + 1;
		dvi_clk <= dvi_clk + 1'b1;
//		if(dvi_clk)
			vga_clk <= ~vga_clk;
	end
		

always @(posedge vga_clk or negedge rst_n)
	if(!rst_n) xcnt <= 12'd0;
	else if(xcnt >= VGA_HTT) xcnt <= 12'd0;
	else xcnt <= xcnt+1'b1;

always @(posedge vga_clk or negedge rst_n)
	if(!rst_n) ycnt <= 12'd0;
	else if(xcnt == VGA_HTT) begin
		if(ycnt >= VGA_VTT) ycnt <= 12'd0;
		else ycnt <= ycnt+1'b1;
	end
	else ;
		
//-----------------------------------------------------------
	//hsy and vsy generate	
always @(posedge vga_clk or negedge rst_n)
	if(!rst_n) vga_hsy <= 1'b0;
	else if(xcnt < VGA_HST) vga_hsy <= 1'b1;
	else vga_hsy <= 1'b0;
	
always @(posedge vga_clk or negedge rst_n)
	if(!rst_n) vga_vsy <= 1'b0;
	else if(ycnt < VGA_VST) vga_vsy <= 1'b1;
	else vga_vsy <= 1'b0;	
	
//-----------------------------------------------------------	
	//vga valid signal generate
reg[7:0] x;
reg[7:0] grayscale;

always @(posedge vga_clk or negedge rst_n)
	if(!rst_n) vga_de <= 1'b0;
	else if((xcnt >= (VGA_HST+VGA_HBP)) && (xcnt < (VGA_HST+VGA_HBP+VGA_HVT))
				&& (ycnt >= (VGA_VST+VGA_VBP)) && (ycnt < (VGA_VST+VGA_VBP+VGA_VVT)))
	begin
		vga_de <= 1'b1;
		x = (xcnt - (VGA_HST+VGA_HBP))/4;
//		 grayscale = ((ycnt - (VGA_VST+VGA_VBP)) / 4) * 8 + (xcnt - (VGA_HST+VGA_HBP))/8;
		if (((ycnt - (VGA_VST+VGA_VBP)) >= 0) && ((ycnt - (VGA_VST+VGA_VBP)) < 150))
			grayscale = 0 * 8 * 8 + ((xcnt - (VGA_HST+VGA_HBP)) / 128) * 8;
		else if (((ycnt - (VGA_VST+VGA_VBP)) >= 150) && ((ycnt - (VGA_VST+VGA_VBP)) < 300))
			grayscale = 1 * 8 * 8 + ((xcnt - (VGA_HST+VGA_HBP)) / 128) * 8;
		else if (((ycnt - (VGA_VST+VGA_VBP)) >= 300) && ((ycnt - (VGA_VST+VGA_VBP)) < 450))
			grayscale = 2 * 8 * 8 + ((xcnt - (VGA_HST+VGA_HBP)) / 128) * 8;
		else if (((ycnt - (VGA_VST+VGA_VBP)) >= 450) && ((ycnt - (VGA_VST+VGA_VBP)) < 600))
			grayscale = 3 * 8 * 8 + ((xcnt - (VGA_HST+VGA_HBP)) / 128) * 8;
	end
	else vga_de <= 1'b0;

reg[7:0] val_r;
reg[7:0] val_g;
reg[7:0] val_b;
reg[4:0] switch;


always @(posedge clk or negedge rst_n)
	if(!rst_n)begin 
		led <= 1'b1;
		switch <= 4'b0;
	end
	else if (key_out)
	begin
			led <= ~led;
			switch <= switch + 1'b1;
			if (switch == 4'd8)
				switch <= 4'd0;
	end
	else
		led <= led;

	//end

/*
always @(posedge vga_clk or negedge rst_n)
	if(!rst_n) begin
		x <= 11'b0;
		y <= 11'b0;
	end
	else if (x === 3'd0)begin*/
	
always @(posedge clk or negedge rst_n)
	if(!rst_n) begin
		val_r <= 8'b0;
		val_g <= 8'b0;
		val_b <= 8'b0;
	end
	else if (switch === 4'd0)begin
		val_r <= 8'b11111111;
		val_g <= 8'b0;
		val_b <= 8'b0;
	end
	else if (switch === 4'd1)begin
		val_r <= 8'b0;
		val_g <= 8'b11111111;
		val_b <= 8'b0;
	end
	else if (switch === 4'd2)begin
		val_r <= 8'b0;
		val_g <= 8'b0;
		val_b <= 8'b11111111;
	end
	else if (switch === 4'd3)begin
		val_r <= 8'b0;
		val_g <= 8'b0;
		val_b <= 8'b0;
	end
	else if (switch === 4'd4)begin
		val_r <= 8'b11111111;
		val_g <= 8'b11111111;
		val_b <= 8'b11111111;
	end
	else if (switch === 4'd5)begin
		val_r <= x;
		val_g <= 8'b0;
		val_b <= 8'b0;
	end
	else if (switch === 4'd6)begin
		val_r <= 8'b0;
		val_g <= x;
		val_b <= 8'b0;
	end
	else if (switch === 4'd7)begin
		val_r <= 8'b0;
		val_g <= 8'b0;
		val_b <= x;
	end
	else if (switch === 4'd8)begin
		val_r <= grayscale;
		val_g <= grayscale;
		val_b <= grayscale;
	end

assign vga_r = val_r;
assign vga_g = val_g;
assign vga_b = val_b;

/*
assign B0 = 1'b1;//count[24];
assign B1 = 1'b1;//[24];
assign B2 = 1'b1;//[24];
assign B3 = 1'b1;//[24];
assign B4 = 1'b1;//[24];
assign B5 = 1'b1;//[24];
assign B6 = 1'b1;//[24];
assign B7 = 1'b1;//[24]; 

assign clk_sync = count[0];
assign hsyhc = (hs_count < 272)? 1'b0:1'b1;
assign de = enable;
assign vsync = (vs_count < 6)? 1'b0:1'b1;
*/
//assign led = count[24];
assign led1 = count[24];
//assign led2 = count[28];
//assign led3 = count[28];

endmodule
