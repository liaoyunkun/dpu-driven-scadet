`timescale 1ps/1ps
`default_nettype wire

module min_2_1 #(
    parameter DATA_NUM = 16,
    parameter DATA_WIDTH = 8,
    parameter IDX_X_WIDTH = 3,
    parameter IDX_Y_WIDTH = 4
)(
    input clk,
    input rst_n,
    input i_val,
    input d0_val,
    input [DATA_WIDTH-1: 0] d0,
    input [IDX_X_WIDTH-1: 0] d0_x,
    input [IDX_Y_WIDTH-1: 0] d0_y,
    input d1_val,
    input [DATA_WIDTH-1: 0] d1,
    input [IDX_X_WIDTH-1: 0] d1_x,
    input [IDX_Y_WIDTH-1: 0] d1_y,
    output reg o_val,
    output reg res_val,
    output reg [DATA_WIDTH-1: 0] res_d,
    output reg [IDX_X_WIDTH-1: 0] res_x,
    output reg [IDX_Y_WIDTH-1: 0] res_y 
);
    always @(posedge clk or negedge rst_n)
        begin
            if (~rst_n)
                begin
                    o_val <= 1'b0;
                end
            else
                begin
                    o_val <= i_val;
                end
        end

    always @(posedge clk or negedge rst_n)
        begin
            if (~rst_n)
                begin
                    res_val <= 1'b0;
                end
            else   
                begin
                    res_val <= d0_val | d1_val;
                end
        end

    always @(posedge clk or negedge rst_n)
        begin
            if (~rst_n)
                begin
                    res_d <= 0;
                    res_x <= 0;
                    res_y <= 0;
                end
            else if (d0_val && !d1_val)
                begin
                    res_d <= d0;
                    res_x <= d0_x;
                    res_y <= d0_y;
                end
            else if (d1_val && !d0_val)
                begin
                    res_d <= d1;
                    res_x <= d1_x;
                    res_y <= d1_y;
                end
            else if (d0_val && d1_val)
                begin
                    if (d0 <= d1)
                        begin
                            res_d <= d0;
                            res_x <= d0_x;
                            res_y <= d0_y;
                        end
                    else
                        begin
                            res_d <= d1;
                            res_x <= d1_x;
                            res_y <= d1_y;
                        end
                end
        end
endmodule