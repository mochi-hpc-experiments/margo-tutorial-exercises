# Margo tutorial exercises

This repository contains exercises meant to accompany Mochi tutorials.
These tutorials focus on using the C language, with the Margo library.
They revolve around the development of a phone book microservice, i.g.
a service that associates names (null-terminated strings) with phone
numbers (which will be represented as `uint64_t` values).
Only the first exercise uses the code present in this repository,
however this README provides instructions for all three exercises.
A equivalent repository for C++ projects is available [here]().

# Initial setup

TODO: instructions for setting up the docker container.

## Exercise 1: simple RPC and RDMA using Margo

The *src* directory provides a *client.c* client code, a *server.c*
server code, a *types.h* header defining RPC types, and a *phonebook.h*.
file containing a (very naive) implementation of a phonebook.

In this exercise we will make the server manage a phonebook and
service two kinds of RPCs: adding a new entry, and looking up a phone
number associated with a name.

* Looking at the API in *phonebook.h*, edit *server.c* to add the creation
of a phonebook object and its destruction when the server terminates.

* Our two RPCs, which we will call "insert" and "lookup", will need
argument and return types. Edit the *types.h* file to add the necessary
type definitions for these RPCs (`insert_in_t`, `insert_out_t`, `lookup_in_t`
and `lookup_out_t`. _Hint: Mercury represents null-terminated strings with
the type `hg_string_t`_. Note: while the insertion operation does not
technically return anything, it is still advised to make all RPCs return at
least a `uint32_t` error code to inform the sender of the success of the
operation.

* Edit *server.c* to add the definitions and declarations of the ULTs for
our two RPCs. Feel free to copy/paste and modify the existing `sum` RPC.
Don't forgot to register your RPCs with the margo instance in main!

* Edit *client.c* and use the existing code as an example to (1) register
the two RPCs here as well, and (2) define two `insert` and `lookup` functions
that will take a `margo_instance_id` alongside the necessary arguments
to create an `hg_handle_t`, forward it to the server with the proper arguments,
and receive the response.

* Try out your code by calling `insert` and `lookup` a few times in the client.

_Bonus:_ If you have time, add a `lookup_multi` RPC that uses RDMA to return the
phone numbers associated with multiple names at once (in practice this would
still be too little data to call for the use of RDMA, but we will just pretent).
For this, you may use the example [here](https://mochi.readthedocs.io/en/latest/margo/04_bulk.html).

Here are some tips for this part:

* Your `lookup_multi` client-side function could take the number of names
as a `uint32_t` and the list of names to look up as an array of null-terminated
strings (`const char* const*`), as well as an output array of `uint64_t`.
* You will need to create two bulk handles on the client and two on the server.
On the client, the first will expose the names as read-only (remember that
`margo_bulk_create` can take a list of non-contiguous segments, but you will
need to use `strlen(...)+1` as the size of each segment to keep the null terminator
of each name), and the second will expose the output array as write only.
* You will need to transfer the two bulk handles in the RPC arguments,
and since names can have a varying size, you will have to also transfer the
total size of the bulk handle wrapping names, so that the server knows
how much memory to allocate for its local buffer.
* On the server side, you will need to allocate two buffers; one to receive
the names via a pull operation, the other to send the phone numbers via a push.
* You will need to create two `hg_bulk_t` to expose these buffers.
* After having transferred the names, they will be in the server's contiguous
buffers. You can rely on the null-terminators to know where one name ends and
the next starts.

## Exercise 2: a proper phonebook Mochi component


