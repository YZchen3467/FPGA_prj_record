module spi_decoder(
    input               spi_clk,
    input               cs_n,
    input               mosi,

    output reg          miso,

    // output interface
    output wire         r_w_flag,
    output wire [1:0]   channel_sel,
    output wire [7:0]   address,
    output wire [21:0]  spi_cmd,
    output wire [5:0]   slv_cnt_dbg         // Debug signal
);

//================================================================
//    Register
//================================================================
reg [5:0]   cnt;                     // Counter for bit position
reg [21:0]  register_table [0:9];    // Register table with 10 entries, each 22 bits wide
reg [21:0]  data_out;                // Data output register for MISO

reg         read_mode;               // Flag to indicate read mode (1) or write mode (0)
reg [1:0]   channel;
reg [7:0]   addr;                    // Address register (8 bits)
reg [21:0]  cmd;

//================================================================
//    Wire
//================================================================
wire spi_clk_buf;
wire spi_clk_global;

//================================================================
//    Assign
//================================================================
assign slv_cnt_dbg = cnt;

assign r_w_flag = read_mode;
assign channel_sel = (cnt >= 3) ? channel:2'd0;
assign address = (cnt >= 11) ? addr:8'd0;
assign spi_cmd = (!read_mode && cnt >= 33) ? cmd:22'd0;

//================================================================
//    Clock Buffer
//================================================================
IBUF spi_clk_ibuf (
       .O(spi_clk_buf),    // Clock output
       .I(spi_clk)         // Clock input
     );

BUFG spi_clk_bufg(
       .O(spi_clk_global),
       .I(spi_clk_buf)
     );

//================================================================
//    Counter Logic
//================================================================
always @(posedge spi_clk_global or posedge cs_n) begin
    if(cs_n)                    cnt <= 6'd0;
    else if(cnt < 6'd33)        cnt <= cnt + 6'd1; 
end

//================================================================
//    Address and R/W Mode Capture Logic
//================================================================
always @(posedge spi_clk or posedge cs_n) begin
    if(cs_n) begin
        read_mode <= 1'b0;      // Clear read mode flag
        addr <= 8'd0;           // Clear address register
        data_out <= 22'b0;      // Clear data output register
    end
    else begin
        if(cnt == 0)                                    read_mode <= mosi;
        else if(cnt >= 1 && cnt <= 2)                   channel <= {channel[0], mosi};
        else if(cnt >= 3 && cnt <= 10)                  addr <= {addr[6:0], mosi};                  // Shift in address bits
        else if(cnt >= 11 && cnt <= 32)                 cmd  <= {cmd[20:0],mosi};
        // else if (cnt == 11 && read_mode)                data_out <= register_table[addr[3:0]];   // Use lower 4 bits for 10-entry table
        // else if (read_mode && cnt >= 11 && cnt <= 33)   data_out <= {data_out[20:0], 1'b0};      // Shift data_out left
    end
end

//================================================================
//    MISO Output Logic (Read Operation)
//================================================================
always @(posedge spi_clk or posedge cs_n) begin
    if (cs_n) miso <= 1'b0;                           // Clear MISO when cs_n is high
    else if (read_mode && cnt >= 11 && cnt <= 32)     // Transmit data_out[21:0] during cnt = 11 to 32 for read operation
        miso <= register_table[addr[3:0]][32-cnt];    // Output MSB first
end

//================================================================
//    Register Table Initialization for Synthesis (10 entries, each 22 bits)
//================================================================
// always @(*) begin
//     // Use direct assignment for constant values to enable synthesis
//     register_table[0] = 22'h06d53e;  
//     register_table[1] = 22'h26d53e;  
//     register_table[2] = 22'h16d53e;  
//     register_table[3] = 22'h36d53e;  
//     register_table[4] = 22'h16d53e;  
//     register_table[5] = 22'h16d53e;  
//     register_table[6] = 22'h06d53f;  
//     register_table[7] = 22'h06d54e;  
//     register_table[8] = 22'h06d56e;  
//     register_table[9] = 22'h06d73e;  
// end

initial begin
    register_table[0] = 22'h06d53e;  
    register_table[1] = 22'h26d53e;
    register_table[2] = 22'h16d53e;
    register_table[3] = 22'h36d53e;
    register_table[4] = 22'h17d53e;
    register_table[5] = 22'h16d53f;
    register_table[6] = 22'h06d55e;
    register_table[7] = 22'h06d54e;
    register_table[8] = 22'h06d56e;
    register_table[9] = 22'h06d73e;
end

endmodule
