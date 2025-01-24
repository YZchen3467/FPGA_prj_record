`timescale 1ns / 1ps

module baudrate#(
    parameter integer BAUDRATE = 115200,
    parameter integer CLK_REF  = 50         // System refclk Mhz
)(
    input uart_clk,
    input uart_rst_p,

    input tx_clk_en,
    input rx_clk_en,

    output tx_baud_clk,
    output rx_baud_clk
);
//================================================================
//    Register 
//================================================================
reg [12:0] tx_cnt, rx_cnt;

//================================================================
//    Parameter
//================================================================
localparam CYCLE = (CLK_REF*1000000) / BAUDRATE;

//================================================================
//    Assign
//================================================================
assign tx_baud_clk = (tx_cnt == 13'd1) ? 1'b1:1'b0;
assign rx_baud_clk = (rx_cnt == CYCLE >> 1'b1) ? 1'b1:1'b0;

//================================================================
//    Baudrate uart_clk gen
//================================================================
/*** tx_cnt for tx_clk gen ***/
always @(posedge uart_clk or posedge uart_rst_p) begin
    if(uart_rst_p)          tx_cnt <= 13'd0;
    else if(tx_clk_en) begin
        if(tx_cnt == CYCLE) tx_cnt <= 13'd0;
        else                tx_cnt <= tx_cnt + 13'd1;
    end
    else                    tx_cnt <= 13'd0;
end

/*** tx_cnt for tx_clk gen ***/
always @(posedge uart_clk or posedge uart_rst_p) begin
    if(uart_rst_p)          rx_cnt <= 13'd0;
    else if(rx_clk_en) begin
        if(rx_cnt == CYCLE) rx_cnt <= 13'd0;
        else                rx_cnt <= rx_cnt + 13'd1;
    end
    else                    rx_cnt <= 13'd0;
end

endmodule
