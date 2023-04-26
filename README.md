# Margo tutorial exercises

This repository contains exercises meant to accompany Mochi tutorials.
These tutorials focus on using the C language, with the Margo library.
They revolve around the development of a phone book microservice, i.g.
a service that associates names (null-terminated strings) with phone
numbers (which will be represented as `uint64_t` values).
Only the first exercise uses the code present in this repository,
however this README provides instructions for all three exercises.
A equivalent repository for C++ projects is available [here]().

## Initial setup

TODO: instructions for setting up the docker container.

## Exercise 1: simple RPC and RDMA using Margo

The *src* directory provides a *client.c* client code, a *server.c*
server code, a *types.h* header defining RPC types, and a *phonebook.h*.
file containing a (very naive) implementation of a phonebook.

In this exercise we will make the server manage a phonebook and
service two kinds of RPCs: adding a new entry, and looking up a phone
number associated with a name.

* Let's start by setting up the spack environment and building the existing
code:

```
$ spack env create margo-tuto-env spack.yaml
$ spack env activate margo-tuto-env
$ spack install
$ mkdir build
$ cd build
$ cmake ..
$ make
```

This will create the *client* and *server* programs.

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

In the second part of this exercise, we will add a `lookup_multi` RPC that uses
RDMA to return the phone numbers associated with multiple names at once (in
practice this would still be too little data to call for the use of RDMA, but
we will just pretent). For this, you may use the example
[here](https://mochi.readthedocs.io/en/latest/margo/04_bulk.html).

Here are some tips for this part:

* Your `lookup_multi` client-side function could take the number of names as a
`uint32_t` and the list of names to look up as an array of null-terminated strings
(`const char* const*`), as well as an output array of `uint64_t`.
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

In this exercise, we will use the
[Margo microservice template](https://github.com/mochi-hpc/margo-microservice-template/)
to develop a proper phonebook microservice.

* Click on the green "Use this template" button and select
"Create a new repository". Give the repository a name (e.g. "phonebook").
Put the repository on "private" if you wish, then click on
"Create repository from template".

* Click on Settings > Actions > General, and set the Workflow permissions to
"Read and write permissions", then Save.

* Go back to the root of the code (in your browser), and edit
`initial-setup.json`. Change "alpha" to a service name of your choosing (this
name will also be used to prefix your API functions, so choose something short,
e.g. YP, for yellow pages. In the following, we will assume this is the name you
used). Change "resource" to the name of the resource we are going to manage,
here "phonebook". Click on the green "Commit changes" button.

* Wait a little and refresh the page. A GitHub workflow will have run and setup
your code. _Note: other github workflows will run to test your code and upload a
coverage report to codecov.io whenever you push commits to GitHub. These
workflows will not properly if you have made the repository private, so you may
receive emails from GitHub about some failed workflows. Simply ignore them._

* Clone your code in the docker container, then create a Spack environment
and build the code like you did in Exercise 1. Create a build directory and
build the code. You may want to use the flag `-DENABLE_TESTS=ON` when calling
`cmake` to make sure that the tests are also built.

It is now time to edit the code to make it do what we want. The template provides
the implementation of two RPCs, "hello" and "sum". You will recognize the "sum"
RPC as the same as provided in Exercise 1. This can be helpful to reproduce
what you have coded in Exercise 1 in the context of this new exercise.

* *include/YP-client.h* contains the functions required to create and free
a `YP_client` object, which will be used to register the RPCs and contact servers.
There is nothing to modify in this file.

* *include/YP-phonebook.h* declares a `YP_phonebook_handle_t` type, which
represents a phonebook managed by a remote server. This file is also where
the client interface will be defined. Go ahead and add declarations for
a `YP_insert` and a `YP_lookup` functions, following what you did in Ex 1.

* *src/client.h* contains the definition of the `YP_client` structure.
Go ahead and add two `hg_id_t` RPC ids to represent the insert and lookup.

* Before looking further into the client implementation, open *src/types.h*
and add the type definitions for our RPCs (`insert_in_t`, insert_out_t`,
`lookup_in_t`, `lookup_out_t`, etc.).

* *src/client.c* contains the implementation of the client-side functions.
In `YP_client_init`, add the registration of our two new RPCs.

* Still in *src/client.c* and using code from Ex 1 (or by copying and
adapting the content of the `YP_compute_sum` function), implement the client-side
function that sends the `insert` and `lookup` RPCs to the server.

At this point, feel free to compile your code to make sure it builds fine
You won't be able to test it yet since there is no server-side implementation
of our RPCs, so let's focus on the server library next.

* *include/YP-backend.h* contains the definition of a backend, i.e. the interface
that a phonebook implementation must satisfy. Add the proper `insert` and `lookup`
function pointers to this structure.

* *src/dummy/dummy-backend.c* contains a "dummy" implementation of such a backend.
Go ahead and add the implementation of the `insert` and `lookup` functions.

* *src/provider.h* contains the state of a phonebook provider. Edit the *YP_provider*
structure to add the `hg_id_t` for our two new RPCs, just like you did for the
client.

* *src/provider.c* contains the implementation of the provider functions. Find
the `YP_provider_register` function and add the registration of your new RPCs
by taking the existing RPCs as examples. Such registration involves (1) calling
`MARGO_REGISTER_PROVIDER` with the appropriate arguments, (2) calling
`margo_register_data` to associate the provider with the RPC, and (3) setting
the RPC ID in the provider structure.

* Still in *src/provider.c*, find the `YP_finalize_provider` function and
add the calls necessary to deregister the two new RPCs.

* We can now implement the functions that will handle the RPCs. In the same file,
find the `YP_sum_ult` function, copy it (including the `DEFINE_MARGO_RPC_HANDLER`
line that follows it) and edit it to transform it into a `YP_insert_ult` function,
then do the same with a `YP_lookup_ult` function.

At this point, you can make sure your code builds fine. Your microservice is ready!
If you have time, feel free to look into the *tests* folder, in particular the
*test-client.c* file, and edit it (replacing calls to the "sum" RPC) to try out
your new functionalities.

In practice, the next steps at this point would be to (1) add more tests,
(2) remove everything related to the "hello" and "sum" RPCs (because obviously
a phonebook is not a calculator), and (3) implement more complex backends
by copying the code of the "dummy" backend and changing it to use external
libraries or more complicated implementations.
