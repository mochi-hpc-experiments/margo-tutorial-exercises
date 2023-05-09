#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include "types.h"

typedef struct {
    int num_rpcs;
    /* (1) add a phonebook field here */
} server_data;

static void sum(hg_handle_t h);
DECLARE_MARGO_RPC_HANDLER(sum)
/* (5) Add the declaration of insert, lookup, and use DECLARE_MARGO_RPC_HANDLER
 * to generate the declaration for their wrappers as well. */

int main(int argc, char** argv)
{
    if(argc != 2) {
        fprintf(stderr, "Usage: %s <protocol>\n", argv[0]);
        exit(-1);
    }

    margo_instance_id mid = margo_init(argv[1], MARGO_SERVER_MODE, 0, 0);
    assert(mid);

    margo_set_log_level(mid, MARGO_LOG_INFO);

    server_data svr_data = {
        .num_rpcs = 0
        /* (2) initialize your phonebook field here*/
    };

    hg_addr_t my_address;
    margo_addr_self(mid, &my_address);
    char addr_str[128];
    size_t addr_str_size = 128;
    margo_addr_to_string(mid, addr_str, &addr_str_size, my_address);
    margo_addr_free(mid,my_address);

    margo_info(mid, "Server running at address %s", addr_str);
    margo_info(mid, "Copy this address to pass to clients");

    hg_id_t rpc_id = MARGO_REGISTER(mid, "sum", sum_in_t, sum_out_t, sum);
    margo_register_data(mid, rpc_id, &svr_data, NULL);
    /* (7) Call MARGO_REGISTER for your insert and lookup RPCs.
     * Don't forget to also call margo_register_data to attach svr_data
     * to the handlers. */

    margo_wait_for_finalize(mid);

    /* (3) destroy your phonebook here */

    return 0;
}

static void sum(hg_handle_t h)
{
    hg_return_t ret;

    sum_in_t in;
    sum_out_t out;

    margo_instance_id mid = margo_hg_handle_get_instance(h);
    margo_set_log_level(mid, MARGO_LOG_INFO);

    const struct hg_info* info = margo_get_info(h);
    server_data* svr_data = (server_data*)margo_registered_data(mid, info->id);

    ret = margo_get_input(h, &in);
    assert(ret == HG_SUCCESS);

    out.ret = in.x + in.y;
    margo_info(mid, "Computed %d + %d = %d", in.x, in.y, out.ret);

    ret = margo_respond(h, &out);
    assert(ret == HG_SUCCESS);

    ret = margo_free_input(h, &in);
    assert(ret == HG_SUCCESS);

    ret = margo_destroy(h);
    assert(ret == HG_SUCCESS);

    svr_data->num_rpcs += 1;
}
DEFINE_MARGO_RPC_HANDLER(sum)

/* (6) Add the definition of insert and lookup here, following the sum example.
 * Don't forget to call DEFINE_MARGO_RPC_HANDLER to also define their wrappers! */
