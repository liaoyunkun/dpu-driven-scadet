`timescale 1ps/1ps
`default_nettype wire

module parallel_com #(
    parameter DATA_NUM = 7,
    parameter DATA_WIDTH = 42,
    parameter IDX_WIDTH = 3
)(
    input clk,
    input rst_n,
    input d_val,
    input [DATA_WIDTH-1: 0] d0,
    input [DATA_WIDTH-1: 0] d1,
    input [DATA_WIDTH-1: 0] d2,
    input [DATA_WIDTH-1: 0] d3,
    input [DATA_WIDTH-1: 0] d4,
    input [DATA_WIDTH-1: 0] d5,
    input [DATA_WIDTH-1: 0] d6,
    input [DATA_WIDTH-1: 0] ref_d,
    input [IDX_WIDTH-1: 0] d_val_cnt,
    output reg res_val,
    output reg res_hit,
    output reg [IDX_WIDTH-1: 0] res_idx 
);
    wire [DATA_NUM-1: 0] com_res;
    reg [DATA_NUM-1: 0] val;
    wire [DATA_NUM-1: 0] com_res_filter;


    assign com_res[0] = (d0 == ref_d)? 1'b1 : 1'b0; 
    assign com_res[1] = (d1 == ref_d)? 1'b1 : 1'b0; 
    assign com_res[2] = (d2 == ref_d)? 1'b1 : 1'b0; 
    assign com_res[3] = (d3 == ref_d)? 1'b1 : 1'b0; 
    assign com_res[4] = (d4 == ref_d)? 1'b1 : 1'b0; 
    assign com_res[5] = (d5 == ref_d)? 1'b1 : 1'b0; 
    assign com_res[6] = (d6 == ref_d)? 1'b1 : 1'b0; 
    
    always @(ref_d)
        begin
            case(ref_d)
            3'd1:
                begin
                    val = 7'b000_0001;
                end
            3'd2:
                begin
                    val = 7'b000_0011;
                end
            3'd3:
                begin
                    val = 7'b000_0111;
                end
            3'd4:
                begin
                    val = 7'b000_1111;
                end
            3'd5:
                begin
                    val = 7'b001_1111;
                end
            3'd6:
                begin
                    val = 7'b011_1111;
                end
            3'd7:
                begin
                    val = 7'b111_1111;
                end
            default:
                begin
                    val = 7'b000_0000;
                end
            endcase
        end

    assign com_res_filter = com_res & val;

    always @(posedge clk or negedge rst_n) 
        begin
            if (~rst_n)
                begin
                    res_val <= 1'b0;
                    res_hit <= 1'b0;
                    res_idx <= 0;
                end
            else if (d_val)
                begin
                    case(com_res_filter)
                    7'b000_0000: 
                        begin
                            res_val <= 1'b1;
                            res_hit <= 1'b0;
                            res_idx <= 0;
                        end
                    7'b000_0001: 
                        begin
                            res_val <= 1'b1;
                            res_hit <= 1'b1;
                            res_idx <= 0;
                        end
                    7'b000_0010: 
                        begin
                            res_val <= 1'b1;
                            res_hit <= 1'b1;
                            res_idx <= 1;
                        end
                    7'b000_0100: 
                        begin
                            res_val <= 1'b1;
                            res_hit <= 1'b1;
                            res_idx <= 2;
                        end
                    7'b000_1000: 
                        begin
                            res_val <= 1'b1;
                            res_hit <= 1'b1;
                            res_idx <= 3;
                        end
                    7'b001_0000: 
                        begin
                            res_val <= 1'b1;
                            res_hit <= 1'b1;
                            res_idx <= 4;
                        end
                    7'b010_0000: 
                        begin
                            res_val <= 1'b1;
                            res_hit <= 1'b1;
                            res_idx <= 5;
                        end
                    7'b100_0000: 
                        begin
                            res_val <= 1'b1;
                            res_hit <= 1'b1;
                            res_idx <= 6;
                        end
                    default:
                        begin
                            res_val <= 1'b1;
                            res_hit <= 1'b0;
                            res_idx <= 0;
                        end
                    endcase
                end
        end
endmodule