`timescale 1ns / 1ps

module uart_rxd(
    input       uart_clk,
    input       uart_rst_p,
    
    input       uart_rx_enable,
    input       rx_baud_clk,
    input       rs232_rxd,

    output reg  [7:0] para_data,
    output reg  rx_clk_en,
    output reg  rx_done
    );
//================================================================
//    Wire
//================================================================
wire rs232_rxd_neg; 

//================================================================
//    Register 
//================================================================
reg rx232_rxd_reg0, rx232_rxd_reg1, rx232_rxd_reg2, rx232_rxd_reg3;

reg receiving;

reg [3:0] curr_state, next_state;
reg [7:0] para_data_reg;

//================================================================
//    Parameter
//================================================================
parameter IDLE  = 4'd0, BIT0  = 4'd1, BIT1  = 4'd2, 
          BIT2  = 4'd3, BIT3  = 4'd4, BIT4  = 4'd5, 
          BIT5  = 4'd6, BIT6  = 4'd7, BIT7  = 4'd8, 
          STOP  = 4'd9;

//================================================================
//    2-stage delay for eliminated metastable
//================================================================
always @(posedge uart_clk or posedge uart_rst_p ) begin
    if(uart_rst_p) begin
        rx232_rxd_reg0 <= 1'b0;  
        rx232_rxd_reg1 <= 1'b0;  
        rx232_rxd_reg2 <= 1'b0;  
        rx232_rxd_reg3 <= 1'b0;
    end
    else begin
        rx232_rxd_reg0 <= rs232_rxd     ;
        rx232_rxd_reg1 <= rx232_rxd_reg0; 
        rx232_rxd_reg2 <= rx232_rxd_reg1; 
        rx232_rxd_reg3 <= rx232_rxd_reg2; 
    end   
end

//================================================================
//    Assign
//================================================================
assign rs232_rxd_neg = ~rx232_rxd_reg2 & rx232_rxd_reg3;

//================================================================
//    Receiving siganl gen
//================================================================
always @(posedge uart_clk or posedge uart_rst_p ) begin: receiving_block
    if(uart_rst_p || rx_done)                   receiving <= 1'b0;
    else if(uart_rx_enable && rs232_rxd_neg)    receiving <= 1'b1;      
end

//================================================================
//    FSM rx signal
//================================================================          
always @(posedge uart_clk or posedge uart_rst_p ) begin
    if(uart_rst_p) curr_state <= IDLE;
    else           curr_state <= next_state;
end

always @(*) begin
    case (curr_state)
    IDLE:   next_state = (receiving && rx_baud_clk) ? BIT0:IDLE;   
    BIT0:   next_state = (receiving && rx_baud_clk) ? BIT1:BIT0 ;    
    BIT1:   next_state = (receiving && rx_baud_clk) ? BIT2:BIT1 ;
    BIT2:   next_state = (receiving && rx_baud_clk) ? BIT3:BIT2 ;
    BIT3:   next_state = (receiving && rx_baud_clk) ? BIT4:BIT3 ;
    BIT4:   next_state = (receiving && rx_baud_clk) ? BIT5:BIT4 ;
    BIT5:   next_state = (receiving && rx_baud_clk) ? BIT6:BIT5 ;
    BIT6:   next_state = (receiving && rx_baud_clk) ? BIT7:BIT6 ;
    BIT7:   next_state = (receiving && rx_baud_clk) ? STOP:BIT7 ;
    STOP:   next_state = (receiving && rx_baud_clk) ? IDLE:STOP ;
    default: next_state = IDLE;
    endcase
end

always @(posedge uart_clk or posedge uart_rst_p ) begin
    if(uart_rst_p) begin
        para_data_reg <= 8'd0;
        rx_clk_en     <= 1'b0;
        rx_done       <= 1'b0; 
    end
    else if(receiving) begin
        rx_clk_en <= 1'b1;
        if(rx_baud_clk) begin
            case(curr_state)
                IDLE:   begin
                    para_data_reg <= 8'd0; 
                    rx_done       <= 1'b0; 
                end
                BIT0:begin
                    para_data_reg <= rs232_rxd;
                    rx_done   <= 1'b0;
                end
                BIT1:begin
                    para_data_reg <= rs232_rxd;
                    rx_done   <= 1'b0;
                end
                BIT2:begin
                    para_data_reg <= rs232_rxd;
                    rx_done   <= 1'b0;  
                end
                BIT3:begin
                    para_data_reg <= rs232_rxd;
                    rx_done   <= 1'b0;
                end
                BIT4:begin
                    para_data_reg <= rs232_rxd;
                    rx_done   <= 1'b0;  
                end
                BIT5:begin
                    para_data_reg <= rs232_rxd;
                    rx_done   <= 1'b0;
                end
                BIT6:begin
                    para_data_reg <= rs232_rxd;
                    rx_done   <= 1'b0;  
                end
                BIT7:begin
                    para_data_reg <= rs232_rxd;
                    rx_done   <= 1'b0;
                end
                STOP:begin
                    para_data <= para_data_reg;
                    rx_done   <= 1'b1;  
                end 
            endcase
        end
    end
    else begin
        para_data_reg <= 8'd0;
        rx_clk_en     <= 1'b0;
        rx_done       <= 1'b0;
    end
end

endmodule
