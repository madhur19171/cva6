module ariane_testharness_wrapper(
    input  logic                           clk_i,
    input  logic                           rtc_i,
    input  logic                           rst_ni,
    output logic [31:0]                    exit_o
);

    wire [63 : 0]   Router_data_out     [3 : 0];
    wire            Router_valid_out    [3 : 0];
    wire            Router_ready_out    [3 : 0];

    wire [63 : 0]   Router_data_in      [3 : 0];
    wire            Router_valid_in     [3 : 0];
    wire            Router_ready_in     [3 : 0];

    genvar i;


    generate
        for(i = 0; i < 4; i++) begin :ARIANE_TEST_HARNESS_GEN
        ariane_testharness #(.hart_id_in(i)) i_ariane_testharness
        (
            .clk_i(clk_i),
            .rtc_i(rtc_i),
            .rst_ni(rst_ni),
            .exit_o(),

            .data_in(Router_data_out[i]), .valid_in(Router_valid_out[i]), .ready_in(Router_ready_out[i]),
            .data_out(Router_data_in[i]), .valid_out(Router_valid_in[i]), .ready_out(Router_ready_in[i])

        );
        end
    endgenerate

    Mesh22 mesh22 (
    .clk(clk_i), .rst(~rst_ni),
    .Node0_data_in(Router_data_in[0]), .Node0_valid_in(Router_valid_in[0]), .Node0_ready_in(Router_ready_in[0]),
    .Node0_data_out(Router_data_out[0]), .Node0_valid_out(Router_valid_out[0]), .Node0_ready_out(Router_ready_out[0]),

    .Node1_data_in(Router_data_in[1]), .Node1_valid_in(Router_valid_in[1]), .Node1_ready_in(Router_ready_in[1]),
    .Node1_data_out(Router_data_out[1]), .Node1_valid_out(Router_valid_out[1]), .Node1_ready_out(Router_ready_out[1]),

    .Node2_data_in(Router_data_in[2]), .Node2_valid_in(Router_valid_in[2]), .Node2_ready_in(Router_ready_in[2]),
    .Node2_data_out(Router_data_out[2]), .Node2_valid_out(Router_valid_out[2]), .Node2_ready_out(Router_ready_out[2]),

    .Node3_data_in(Router_data_in[3]), .Node3_valid_in(Router_valid_in[3]), .Node3_ready_in(Router_ready_in[3]),
    .Node3_data_out(Router_data_out[3]), .Node3_valid_out(Router_valid_out[3]), .Node3_ready_out(Router_ready_out[3])
  );
    
endmodule