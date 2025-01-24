`timescale 1ns / 1ps

module uart_top(
    input clk_125_p,
    input clk_125_n,
    input rst_p,

    input       rs232_input,    // rs232 rxd
    output wire rs232_output   // rs232 txd
    // output wire [3:0] led_out 
    );
//================================================================
//    Register
//================================================================
reg [7:0] para_data_temp;
reg [31:0] cnt_1s, cnt_1s_temp;
reg uart_tx_enable_temp;

reg [3:0] char_index;
reg [7:0] my_string [0:13];

//================================================================
//    Initialize the string
//================================================================
initial begin
    my_string[0]  = "H";
    my_string[1]  = "e";
    my_string[2]  = "l";
    my_string[3]  = "l";
    my_string[4]  = "o";
    my_string[5]  = " ";
    my_string[6]  = "W";
    my_string[7]  = "o";
    my_string[8]  = "r";
    my_string[9]  = "l";
    my_string[10] = "d";
    my_string[11] = "!";
    my_string[12] = "\n";
end

//================================================================
//    Wire
//================================================================
wire clk_50MHz;
wire uart_rst_p;

/*** baud clk module ***/
wire tx_baud_clk, rx_baud_clk;

/*** uart txd module ***/
wire uart_tx_enable;
wire [7:0] para_data;
wire tx_clk_en;
wire tx_done;

/*** uart rxd module ***/
wire uart_rx_enable;
wire rx_clk_en;
wire rx_done;

//================================================================
//    Assign
//================================================================
assign uart_rst_p = ~locked;

assign uart_tx_enable = uart_tx_enable_temp;
assign para_data = para_data_temp;

//================================================================
//    Data gen
//================================================================
always @(posedge clk_50MHz or posedge uart_rst_p) begin
    if(uart_rst_p) cnt_1s <= 32'd0;
    else           cnt_1s <= cnt_1s_temp;
end

always @(*) begin
    if(uart_rst_p || cnt_1s == 32'd24_999_999) cnt_1s_temp = 32'd0;
    else                                       cnt_1s_temp = cnt_1s + 32'd1;
end

// always @(posedge clk_50MHz or posedge uart_rst_p) begin
//     if(uart_rst_p)  begin
//         char_index     <= 4'd0;
//         para_data_temp <= 8'd0;
//     end              
//     else if(cnt_1s==32'd24_999_999)begin
//         para_data_temp <= my_string[char_index];
//         if(char_index == 4'd12) char_index <= 12'd0;
//         else                    char_index <= char_index + 4'd1;
//     end          
//     else para_data_temp <= para_data_temp;
// end

always @(posedge clk_50MHz or posedge uart_rst_p) begin
    if(uart_rst_p)                  uart_tx_enable_temp <= 1'b0;
    else if(cnt_1s==32'd24_999_999) uart_tx_enable_temp <= 1'b1;
    else                            uart_tx_enable_temp <= 1'b0;
end

//================================================================
//    baudrate module 
//================================================================
baudrate #(
    .BAUDRATE(115200),
    .CLK_REF (50)
) baudrate_inst(
    .uart_clk (clk_50MHz),
    .uart_rst_p(uart_rst_p),
    
    .tx_clk_en(tx_clk_en),
    .rx_clk_en(rx_clk_en),

    .tx_baud_clk (tx_baud_clk),
    .rx_baud_clk (rx_baud_clk)
);

//================================================================
//    uart_txd module
//================================================================
uart_txd uart_txd_inst(
    .uart_clk(clk_50MHz),
    .uart_rst_p(uart_rst_p),

    .uart_tx_enable(uart_tx_enable),
    .tx_baud_clk(tx_baud_clk),
    .para_data(para_data),

    .rs232_txd(rs232_output),
    .tx_clk_en(tx_clk_en),
    .tx_done(tx_done)
);

//================================================================
//    uart_rxd module
//================================================================
uart_rxd uart_rxd_inst(
    .uart_clk(clk_50MHz),
    .uart_rst_p(uart_rst_p),
    
    .uart_rx_enable(1'b1),
    .rx_baud_clk(rx_baud_clk),
    .rs232_rxd(rs232_input),

    .para_data(para_data),
    .rx_clk_en(rx_clk_en),
    .rx_done(rx_done)

);

//================================================================
//    Clock inst 
//================================================================
IBUFGDS ibufgds_inst_0(
    .I  (clk_125_p),
    .IB (clk_125_n),
    .O  (sys_clk_bufg)
);

BUFG BUFG_inst_0 (
    .O(sys_clk), // 1-bit output: Clock output
    .I(sys_clk_bufg)  // 1-bit input: Clock input
 );


 clk_wiz_0 clk_wiz_ints_0
 (
  // Clock out ports
  .clk_out1(clk_50MHz),     // output clk_out1
  // Status and control signals
  .reset(rst_p), // input reset
  .locked(locked),       // output locked
 // Clock in ports
  .clk_in1(sys_clk)      // input clk_in1
);

ila_0 ila_inst_0 (
	.clk(clk_50MHz), // input wire clk

    .probe0(tx_baud_clk),
    .probe1(rs232_output),
    .probe2(cnt_1s),
    .probe3(para_data),
    .probe4(tx_clk_en),
    .probe5(uart_tx_enable_temp)

);

endmodule
