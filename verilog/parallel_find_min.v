`timescale 1ps/1ps
`default_nettype wire

module parallel_find_min #(
    parameter DATA_NUM = 16,
    parameter DATA_WIDTH = 8,
    parameter IDX_X_WIDTH = 3,
    parameter IDX_Y_WIDTH = 4
)(
    input clk,
    input rst_n,
    input d_val,
    input [IDX_X_WIDTH-1: 0] d_x,
    input [DATA_WIDTH-1: 0] d0,
    input [DATA_WIDTH-1: 0] d1,
    input [DATA_WIDTH-1: 0] d2,
    input [DATA_WIDTH-1: 0] d3,
    input [DATA_WIDTH-1: 0] d4,
    input [DATA_WIDTH-1: 0] d5,
    input [DATA_WIDTH-1: 0] d6,
    input [DATA_WIDTH-1: 0] d7,
    input [DATA_WIDTH-1: 0] d8,
    input [DATA_WIDTH-1: 0] d9,
    input [DATA_WIDTH-1: 0] d10,
    input [DATA_WIDTH-1: 0] d11,
    input [DATA_WIDTH-1: 0] d12,
    input [DATA_WIDTH-1: 0] d13,
    input [DATA_WIDTH-1: 0] d14,
    input [DATA_WIDTH-1: 0] d15,
    input ref_d_val,
    input [DATA_WIDTH-1: 0] ref_d,
    input [IDX_X_WIDTH-1: 0] ref_d_x,
    input [IDX_Y_WIDTH-1: 0] ref_d_y,
    input [IDX_Y_WIDTH: 0] d_val_cnt,
    output o_res_val,
    output res_val,
    output [DATA_WIDTH-1: 0] res,
    output [IDX_X_WIDTH-1: 0] res_idx_x,
    output [IDX_Y_WIDTH-1: 0] res_idx_y
);

    reg [DATA_NUM-1: 0] d_i_val;

    wire o_val_s0 [0: 7];
    wire res_val_s0 [0: 7];
    wire [DATA_WIDTH-1: 0] res_d_s0 [0: 7];
    wire [IDX_X_WIDTH-1: 0] res_x_s0 [0: 7];
    wire [IDX_Y_WIDTH-1: 0] res_y_s0 [0: 7];

    wire o_val_s1 [0: 3];
    wire res_val_s1 [0: 3];
    wire [DATA_WIDTH-1: 0] res_d_s1 [0: 3];
    wire [IDX_X_WIDTH-1: 0] res_x_s1 [0: 3];
    wire [IDX_Y_WIDTH-1: 0] res_y_s1 [0: 3];

    wire o_val_s2 [0: 1];
    wire res_val_s2 [0: 1];
    wire [DATA_WIDTH-1: 0]  res_d_s2 [0: 1];
    wire [IDX_X_WIDTH-1: 0] res_x_s2 [0: 1];
    wire [IDX_Y_WIDTH-1: 0] res_y_s2 [0: 1];


    wire o_val_s3;
    wire res_val_s3;
    wire [DATA_WIDTH-1: 0] res_d_s3;
    wire [IDX_X_WIDTH-1: 0] res_x_s3;
    wire [IDX_Y_WIDTH-1: 0] res_y_s3;

    reg pipe_ref_d_val [0:3];
    reg [DATA_WIDTH-1:0] pipe_ref_d [0:3]; 
    reg [IDX_X_WIDTH-1:0] pipe_ref_d_x [0:3];
    reg [IDX_Y_WIDTH-1:0] pipe_ref_d_y [0:3];

    always @(posedge clk or negedge rst_n)
        begin
            if (~rst_n)
                begin
                    pipe_ref_d_val[0] <= 0;
                    pipe_ref_d[0] <= 0;
                    pipe_ref_d_x[0] <= 0;
                    pipe_ref_d_y[0] <= 0;
                end
            else if (d_val)
                begin
                    // load
                    pipe_ref_d_val[0] <= ref_d_val;
                    pipe_ref_d[0] <= ref_d;
                    pipe_ref_d_x[0] <= ref_d_x;
                    pipe_ref_d_y[0] <= ref_d_y;
                end
        end
    
    always @(posedge clk or negedge rst_n)
        begin
            if (~rst_n)
                begin
                    pipe_ref_d_val[1] <= 0;
                    pipe_ref_d[1] <= 0;
                    pipe_ref_d_x[1] <= 0;
                    pipe_ref_d_y[1] <= 0;
                end
            else
                begin
                    pipe_ref_d_val[1] <= pipe_ref_d_val[0];
                    pipe_ref_d[1] <= pipe_ref_d[0];
                    pipe_ref_d_x[1] <= pipe_ref_d_x[0];
                    pipe_ref_d_y[1] <= pipe_ref_d_y[0];
                end
        end

    always @(posedge clk or negedge rst_n)
        begin
            if (~rst_n)
                begin
                    pipe_ref_d_val[2] <= 0;
                    pipe_ref_d[2] <= 0;
                    pipe_ref_d_x[2] <= 0;
                    pipe_ref_d_y[2] <= 0;
                end
            else
                begin
                    pipe_ref_d_val[2] <= pipe_ref_d_val[1];
                    pipe_ref_d[2] <= pipe_ref_d[1];
                    pipe_ref_d_x[2] <= pipe_ref_d_x[1];
                    pipe_ref_d_y[2] <= pipe_ref_d_y[1];
                end
        end

    always @(posedge clk or negedge rst_n)
        begin
            if (~rst_n)
                begin
                    pipe_ref_d_val[3] <= 0;
                    pipe_ref_d[3] <= 0;
                    pipe_ref_d_x[3] <= 0;
                    pipe_ref_d_y[3] <= 0;
                end
            else
                begin
                    pipe_ref_d_val[3] <= pipe_ref_d_val[2];
                    pipe_ref_d[3] <= pipe_ref_d[2];
                    pipe_ref_d_x[3] <= pipe_ref_d_x[2];
                    pipe_ref_d_y[3] <= pipe_ref_d_y[2];
                end
        end

    always @(d_val_cnt)
        begin
            case(d_val_cnt)
            5'd0:
                begin
                    d_i_val = 16'b0000_0000_0000_0000;
                end
            5'd1:
                begin
                    d_i_val = 16'b0000_0000_0000_0001;
                end
            5'd2:
                begin
                    d_i_val = 16'b0000_0000_0000_0011;
                end
            5'd3:
                begin
                    d_i_val = 16'b0000_0000_0000_0111;
                end
            5'd4:
                begin
                    d_i_val = 16'b0000_0000_0000_1111;
                end
            5'd5:
                begin
                    d_i_val = 16'b0000_0000_0001_1111;
                end
            5'd6:
                begin
                    d_i_val = 16'b0000_0000_0011_1111;
                end    
            5'd7:
                begin
                    d_i_val = 16'b0000_0000_0111_1111;
                end
            5'd8:
                begin
                    d_i_val = 16'b0000_0000_1111_1111;
                end 
            5'd9:
                begin
                    d_i_val = 16'b0000_0001_1111_1111;
                end 
            5'd10:
                begin
                    d_i_val = 16'b0000_0011_1111_1111;
                end
            5'd11:
                begin
                    d_i_val = 16'b0000_0111_1111_1111;
                end    
            5'd12:
                begin
                    d_i_val = 16'b0000_1111_1111_1111;
                end
            5'd13:
                begin
                    d_i_val = 16'b0001_1111_1111_1111;
                end
            5'd14:
                begin
                    d_i_val = 16'b0011_1111_1111_1111;
                end
            5'd15:
                begin
                    d_i_val = 16'b0111_1111_1111_1111;
                end
            5'd16:
                begin
                    d_i_val = 16'b1111_1111_1111_1111;
                end 
            default:
                begin
                    d_i_val = 16'b0000_0000_0000_0000;
                end   
            endcase
        end
    //**************Stage 1**************//
    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s0_0(
        .clk         ( clk          ),
        .rst_n       ( rst_n        ),
        .i_val       ( d_val        ),
        .d0_val      ( d_i_val[0]   ),
        .d0          ( d0           ),
        .d0_x        ( d_x          ),
        .d0_y        ( 4'd0         ),
        .d1_val      ( d_i_val[1]   ),
        .d1          ( d1           ),
        .d1_x        ( d_x          ),
        .d1_y        ( 4'd1         ),
        .o_val       ( o_val_s0[0]  ),
        .res_val     ( res_val_s0[0]),
        .res_d       ( res_d_s0[0]  ),
        .res_x       ( res_x_s0[0]  ),
        .res_y       ( res_y_s0[0]  )
    );

    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s0_1(
        .clk         ( clk          ),
        .rst_n       ( rst_n        ),
        .i_val       ( d_val        ),
        .d0_val      ( d_i_val[2]   ),
        .d0          ( d0           ),
        .d0_x        ( d_x          ),
        .d0_y        ( 4'd2         ),
        .d1_val      ( d_i_val[3]   ),
        .d1          ( d1           ),
        .d1_x        ( d_x          ),
        .d1_y        ( 4'd3         ),
        .o_val       ( o_val_s0[1]  ),
        .res_val     ( res_val_s0[1]),
        .res_d       ( res_d_s0[1]  ),
        .res_x       ( res_x_s0[1]  ),
        .res_y       ( res_y_s0[1]  )
    );

    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s0_2(
        .clk         ( clk          ),
        .rst_n       ( rst_n        ),
        .i_val       ( d_val        ),
        .d0_val      ( d_i_val[4]   ),
        .d0          ( d0           ),
        .d0_x        ( d_x          ),
        .d0_y        ( 4'd4         ),
        .d1_val      ( d_i_val[5]   ),
        .d1          ( d1           ),
        .d1_x        ( d_x          ),
        .d1_y        ( 4'd5         ),
        .o_val       ( o_val_s0[2]  ),
        .res_val     ( res_val_s0[2]),
        .res_d       ( res_d_s0[2]  ),
        .res_x       ( res_x_s0[2]  ),
        .res_y       ( res_y_s0[2]  )
    );

    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s0_3(
        .clk         ( clk          ),
        .rst_n       ( rst_n        ),
        .i_val       ( d_val        ),
        .d0_val      ( d_i_val[6]   ),
        .d0          ( d0           ),
        .d0_x        ( d_x          ),
        .d0_y        ( 4'd6         ),
        .d1_val      ( d_i_val[7]   ),
        .d1          ( d1           ),
        .d1_x        ( d_x          ),
        .d1_y        ( 4'd7         ),
        .o_val       ( o_val_s0[3]  ),
        .res_val     ( res_val_s0[3]),
        .res_d       ( res_d_s0[3]  ),
        .res_x       ( res_x_s0[3]  ),
        .res_y       ( res_y_s0[3]  )
    );

    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s0_4(
        .clk         ( clk          ),
        .rst_n       ( rst_n        ),
        .i_val       ( d_val        ),
        .d0_val      ( d_i_val[8]   ),
        .d0          ( d0           ),
        .d0_x        ( d_x          ),
        .d0_y        ( 4'd8         ),
        .d1_val      ( d_i_val[9]   ),
        .d1          ( d1           ),
        .d1_x        ( d_x          ),
        .d1_y        ( 4'd9         ),
        .o_val       ( o_val_s0[4]  ),
        .res_val     ( res_val_s0[4]),
        .res_d       ( res_d_s0[4]  ),
        .res_x       ( res_x_s0[4]  ),
        .res_y       ( res_y_s0[4]  )
    );

    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s0_5(
        .clk         ( clk          ),
        .rst_n       ( rst_n        ),
        .i_val       ( d_val        ),
        .d0_val      ( d_i_val[10]   ),
        .d0          ( d0           ),
        .d0_x        ( d_x          ),
        .d0_y        ( 4'd10         ),
        .d1_val      ( d_i_val[11]   ),
        .d1          ( d1           ),
        .d1_x        ( d_x          ),
        .d1_y        ( 4'd11         ),
        .o_val       ( o_val_s0[5]  ),
        .res_val     ( res_val_s0[5]),
        .res_d       ( res_d_s0[5]  ),
        .res_x       ( res_x_s0[5]  ),
        .res_y       ( res_y_s0[5]  )
    );

    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s0_6(
        .clk         ( clk          ),
        .rst_n       ( rst_n        ),
        .i_val       ( d_val        ),
        .d0_val      ( d_i_val[12]   ),
        .d0          ( d0           ),
        .d0_x        ( d_x          ),
        .d0_y        ( 4'd12         ),
        .d1_val      ( d_i_val[13]   ),
        .d1          ( d1           ),
        .d1_x        ( d_x          ),
        .d1_y        ( 4'd13         ),
        .o_val       ( o_val_s0[6]  ),
        .res_val     ( res_val_s0[6]),
        .res_d       ( res_d_s0[6]  ),
        .res_x       ( res_x_s0[6]  ),
        .res_y       ( res_y_s0[6]  )
    );

    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s0_7(
        .clk         ( clk          ),
        .rst_n       ( rst_n        ),
        .i_val       ( d_val        ),
        .d0_val      ( d_i_val[14]   ),
        .d0          ( d0           ),
        .d0_x        ( d_x          ),
        .d0_y        ( 4'd14         ),
        .d1_val      ( d_i_val[15]   ),
        .d1          ( d1           ),
        .d1_x        ( d_x          ),
        .d1_y        ( 4'd15         ),
        .o_val       ( o_val_s0[7]  ),
        .res_val     ( res_val_s0[7]),
        .res_d       ( res_d_s0[7]  ),
        .res_x       ( res_x_s0[7]  ),
        .res_y       ( res_y_s0[7]  )
    );
    //**************Stage 2**************//
    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s1_0(
        .clk         ( clk           ),
        .rst_n       ( rst_n         ),
        .i_val       ( o_val_s0[0]   ),
        .d0_val      ( res_val_s0[0] ),
        .d0          ( res_d_s0[0]   ),
        .d0_x        ( res_x_s0[0]   ),
        .d0_y        ( res_y_s0[0]   ),
        .d1_val      ( res_val_s0[1] ),
        .d1          ( res_d_s0[1]   ),
        .d1_x        ( res_x_s0[1]   ),
        .d1_y        ( res_y_s0[1]   ),
        .o_val       ( o_val_s1[0]   ),
        .res_val     ( res_val_s1[0] ),
        .res_d       ( res_d_s1[0]   ),
        .res_x       ( res_x_s1[0]   ),
        .res_y       ( res_y_s1[0]   )
    );

    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s1_1(
        .clk         ( clk           ),
        .rst_n       ( rst_n         ),
        .i_val       ( o_val_s0[2]   ),
        .d0_val      ( res_val_s0[2] ),
        .d0          ( res_d_s0[2]   ),
        .d0_x        ( res_x_s0[2]   ),
        .d0_y        ( res_y_s0[2]   ),
        .d1_val      ( res_val_s0[3] ),
        .d1          ( res_d_s0[3]   ),
        .d1_x        ( res_x_s0[3]   ),
        .d1_y        ( res_y_s0[3]   ),
        .o_val       ( o_val_s1[1]   ),
        .res_val     ( res_val_s1[1] ),
        .res_d       ( res_d_s1[1]   ),
        .res_x       ( res_x_s1[1]   ),
        .res_y       ( res_y_s1[1]   )
    );

    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s1_2(
        .clk         ( clk           ),
        .rst_n       ( rst_n         ),
        .i_val       ( o_val_s0[4]   ),
        .d0_val      ( res_val_s0[4] ),
        .d0          ( res_d_s0[4]   ),
        .d0_x        ( res_x_s0[4]   ),
        .d0_y        ( res_y_s0[4]   ),
        .d1_val      ( res_val_s0[5] ),
        .d1          ( res_d_s0[5]   ),
        .d1_x        ( res_x_s0[5]   ),
        .d1_y        ( res_y_s0[5]   ),
        .o_val       ( o_val_s1[2]   ),
        .res_val     ( res_val_s1[2] ),
        .res_d       ( res_d_s1[2]   ),
        .res_x       ( res_x_s1[2]   ),
        .res_y       ( res_y_s1[2]   )
    );

    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s1_3(
        .clk         ( clk           ),
        .rst_n       ( rst_n         ),
        .i_val       ( o_val_s0[6]   ),
        .d0_val      ( res_val_s0[6] ),
        .d0          ( res_d_s0[6]   ),
        .d0_x        ( res_x_s0[6]   ),
        .d0_y        ( res_y_s0[6]   ),
        .d1_val      ( res_val_s0[7] ),
        .d1          ( res_d_s0[7]   ),
        .d1_x        ( res_x_s0[7]   ),
        .d1_y        ( res_y_s0[7]   ),
        .o_val       ( o_val_s1[3]   ),
        .res_val     ( res_val_s1[3] ),
        .res_d       ( res_d_s1[3]   ),
        .res_x       ( res_x_s1[3]   ),
        .res_y       ( res_y_s1[3]   )
    );
    //*************Stage 3*******************//
    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s2_0(
        .clk         ( clk           ),
        .rst_n       ( rst_n         ),
        .i_val       ( o_val_s1[0]   ),
        .d0_val      ( res_val_s1[0] ),
        .d0          ( res_d_s1[0]   ),
        .d0_x        ( res_x_s1[0]   ),
        .d0_y        ( res_y_s1[0]   ),
        .d1_val      ( res_val_s1[1] ),
        .d1          ( res_d_s1[1]   ),
        .d1_x        ( res_x_s1[1]   ),
        .d1_y        ( res_y_s1[1]   ),
        .o_val       ( o_val_s2[0]   ),
        .res_val     ( res_val_s2[0] ),
        .res_d       ( res_d_s2[0]   ),
        .res_x       ( res_x_s2[0]   ),
        .res_y       ( res_y_s2[0]   )
    );

    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s2_1(
        .clk         ( clk           ),
        .rst_n       ( rst_n         ),
        .i_val       ( o_val_s1[2]   ),
        .d0_val      ( res_val_s1[2] ),
        .d0          ( res_d_s1[2]   ),
        .d0_x        ( res_x_s1[2]   ),
        .d0_y        ( res_y_s1[2]   ),
        .d1_val      ( res_val_s1[3] ),
        .d1          ( res_d_s1[3]   ),
        .d1_x        ( res_x_s1[3]   ),
        .d1_y        ( res_y_s1[3]   ),
        .o_val       ( o_val_s2[1]   ),
        .res_val     ( res_val_s2[1] ),
        .res_d       ( res_d_s2[1]   ),
        .res_x       ( res_x_s2[1]   ),
        .res_y       ( res_y_s2[1]   )
    );
    //**************Stage 4*************//
    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s3_0(
        .clk         ( clk           ),
        .rst_n       ( rst_n         ),
        .i_val       ( o_val_s2[0]   ),
        .d0_val      ( res_val_s2[0] ),
        .d0          ( res_d_s2[0]   ),
        .d0_x        ( res_x_s2[0]   ),
        .d0_y        ( res_y_s2[0]   ),
        .d1_val      ( res_val_s2[1] ),
        .d1          ( res_d_s2[1]   ),
        .d1_x        ( res_x_s2[1]   ),
        .d1_y        ( res_y_s2[1]   ),
        .o_val       ( o_val_s3      ),
        .res_val     ( res_val_s3    ),
        .res_d       ( res_d_s3      ),
        .res_x       ( res_x_s3      ),
        .res_y       ( res_y_s3      )
    );
    //**************Stage 5***************//
    min_2_1#(
        .DATA_NUM    ( 16 ),
        .DATA_WIDTH  ( 8 ),
        .IDX_X_WIDTH ( 3 ),
        .IDX_Y_WIDTH ( 4 )
    )u_min_2_1_s4_0(
        .clk         ( clk              ),
        .rst_n       ( rst_n            ),
        .i_val       ( o_val_s3         ),
        .d0_val      ( res_val_s3       ),
        .d0          ( res_d_s3         ),
        .d0_x        ( res_x_s3         ),
        .d0_y        ( res_y_s3         ),
        .d1_val      ( pipe_ref_d_val[3]),
        .d1          ( pipe_ref_d[3]    ),
        .d1_x        ( pipe_ref_d_x[3]  ),
        .d1_y        ( pipe_ref_d_y[3]  ),
        .o_val       ( o_res_val        ),
        .res_val     ( res_val          ),
        .res_d       ( res              ),
        .res_x       ( res_idx_x        ),
        .res_y       ( res_idx_y        )
    );

endmodule