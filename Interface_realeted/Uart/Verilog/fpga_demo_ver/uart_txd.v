`timescale 1ns / 1ps

module uart_txd(
    input       uart_clk,
    input       uart_rst_p,
    
    input       uart_tx_enable,
    input       tx_baud_clk,
    input [7:0] para_data,

    output reg rs232_txd,
    output reg tx_clk_en,
    output reg tx_done
    );
//================================================================
//    Register 
//================================================================
reg transmiting;        // tell the state machine, the data is r_transmiting
                        // do not do anyting in r_transmiting state.

/*** State def ***/
reg [3:0] curr_state, next_state;

//================================================================
//    Parameter
//================================================================
parameter IDLE  = 4'd0, BIT0  = 4'd1, BIT1  = 4'd2, 
          BIT2  = 4'd3, BIT3  = 4'd4, BIT4  = 4'd5, 
          BIT5  = 4'd6, BIT6  = 4'd7, BIT7  = 4'd8, 
          STOP  = 4'd9;

//================================================================
//    Transmiting signal gen
//================================================================          
always @(posedge uart_clk or posedge uart_rst_p ) begin: transmiting_block
    if(uart_rst_p || tx_done)   transmiting <= 1'b0;
    else if(uart_tx_enable)          transmiting <= 1'b1;      
end

//================================================================
//    FSM tx signal
//================================================================          
always @(posedge uart_clk or posedge uart_rst_p ) begin: state
    if(uart_rst_p) curr_state <= IDLE;
    else           curr_state <= next_state;
end

always @(*) begin: state_transfer_block
    case(curr_state)
        IDLE:   next_state = (transmiting && tx_baud_clk) ? BIT0:IDLE;   
        BIT0:   next_state = (transmiting && tx_baud_clk) ? BIT1:BIT0 ;    
        BIT1:   next_state = (transmiting && tx_baud_clk) ? BIT2:BIT1 ;
        BIT2:   next_state = (transmiting && tx_baud_clk) ? BIT3:BIT2 ;
        BIT3:   next_state = (transmiting && tx_baud_clk) ? BIT4:BIT3 ;
        BIT4:   next_state = (transmiting && tx_baud_clk) ? BIT5:BIT4 ;
        BIT5:   next_state = (transmiting && tx_baud_clk) ? BIT6:BIT5 ;
        BIT6:   next_state = (transmiting && tx_baud_clk) ? BIT7:BIT6 ;
        BIT7:   next_state = (transmiting && tx_baud_clk) ? STOP:BIT7 ;
        STOP:   next_state = (transmiting && tx_baud_clk) ? IDLE:STOP ;
        default: next_state = IDLE;
    endcase
end

always @(posedge uart_clk or posedge uart_rst_p) begin: FSM_output
    if(uart_rst_p) begin
        rs232_txd <= 1'b1;
        tx_clk_en <= 1'b0;
        tx_done   <= 1'b0;
    end
    else if(transmiting) begin
        tx_clk_en <= 1'b1;
        if(tx_baud_clk) begin
            case (curr_state)
                IDLE:   begin
                    rs232_txd <= 1'b0; 
                    tx_done   <= 1'b0; 
                end
                BIT0:begin
                    rs232_txd <= para_data[0];
                    tx_done   <= 1'b0;
                end
                BIT1:begin
                    rs232_txd <= para_data[1];
                    tx_done   <= 1'b0;
                end
                BIT2:begin
                    rs232_txd <= para_data[2];
                    tx_done   <= 1'b0;  
                end
                BIT3:begin
                    rs232_txd <= para_data[3];
                    tx_done   <= 1'b0;
                end
                BIT4:begin
                    rs232_txd <= para_data[4];
                    tx_done   <= 1'b0;  
                end
                BIT5:begin
                    rs232_txd <= para_data[5];
                    tx_done   <= 1'b0;
                end
                BIT6:begin
                    rs232_txd <= para_data[6];
                    tx_done   <= 1'b0;  
                end
                BIT7:begin
                    rs232_txd <= para_data[7];
                    tx_done   <= 1'b0;
                end
                STOP:begin
                    rs232_txd <= 1'b1;
                    tx_done   <= 1'b1;  
                end 
            endcase
        end
    end
    else begin
        rs232_txd <= 1'b1;
        tx_clk_en <= 1'b0;
        tx_done   <= 1'b0;
    end
end

endmodule
