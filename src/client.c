#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <margo.h>
#include "types.h"

/* (9) Define the insert and lookup functions with the following prototype:
 * void insert(margo_instance_id mid, hg_id_t rpc_id, const char* name, uint64_t number);
 * uint64_t lookup(margo_instance_id mid, hg_id_t rpc_id, const char* name);
 */

int main(int argc, char** argv)
{
    if(argc != 2) {
        fprintf(stderr,"Usage: %s <server address>\n", argv[0]);
        exit(0);
    }

    // this code copies the protocol part of the server address
    // (i.e. address up to the fist column character)
    const char* server_address = argv[1];
    char protocol[32];
    memset(protocol, 0, 32);
    for(int i=0; server_address[i] != '\0' && server_address[i] != ':' && i < 31; ++i)
        protocol[i] = server_address[i];

    margo_instance_id mid = margo_init(protocol, MARGO_CLIENT_MODE, 0, 0);
    margo_set_log_level(mid, MARGO_LOG_INFO);

    hg_id_t sum_rpc_id = MARGO_REGISTER(mid, "sum", sum_in_t, sum_out_t, NULL);
    /* (8) Register the insert and lookup RPCs here */

    hg_addr_t svr_addr;
    margo_addr_lookup(mid, server_address, &svr_addr);

    int i;
    sum_in_t args;
    for(i=0; i<4; i++) {
        args.x = 42+i*2;
        args.y = 42+i*2+1;

        hg_handle_t h;
        margo_create(mid, svr_addr, sum_rpc_id, &h);
        margo_forward(h, &args);

        sum_out_t resp;
        margo_get_output(h, &resp);

        margo_info(mid, "Got response: %d+%d = %d", args.x, args.y, resp.ret);

        margo_free_output(h,&resp);
        margo_destroy(h);
    }

    margo_addr_free(mid, svr_addr);

    margo_finalize(mid);

    return 0;
}

/* (11) Prototype of a possible lookup_multi function:
 * void lookup_multi(margo_instance_id mid, hg_id_t rpc_id,
 *                   uint32_t count, const char* const* names,
 *                   uint64_t* numbers);
 */
