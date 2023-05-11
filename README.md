# Margo tutorial exercises

This repository contains exercises meant to accompany Mochi tutorials.
These tutorials focus on using the C language, with the Margo library.
They revolve around the development of a phonebook microservice, i.e.
a service that associates names (null-terminated strings) with phone
numbers (which will be represented as `uint64_t` values).
Only the first exercise uses the code present in this repository,
however this README provides instructions for all three exercises.
A equivalent repository for C++ projects is available
[here](https://github.com/mochi-hpc-experiments/thallium-tutorial-exercises).

## Initial setup

Please begin by creating a subdirectory called `mochi-tutorial` on your
machine and cloning this hands-on exercise repo into it:

```
mkdir mochi-tutorial
cd mochi-tutorial
git clone https://github.com/mochi-hpc-experiments/margo-tutorial-exercises.git
cd ..
```

The next step is to set up a Mochi development environment.  The most
straightforward way to do this is by creating a Docker container using an
image that is preconfigured for use with the Mochi tutorial hands-on
exercises.  This method is documented as **Option 1** below.  If you prefer
to use native environment, please skip to **Option 2**.

### Option 1 (preferred): create a development environment using Docker

You need to first have Docker installed on your machine; please see the
[Docker installation instructions for your
platform](https://docs.docker.com/get-docker/). Once Docker is installed,
you can use the following commands to download a preconfigured image:

```
$ docker pull carns/mochi-tutorial:latest
$ docker tag carns/mochi-tutorial:latest mochi-tutorial
```

The following run command will instantiate a new container from the
mochi-tutorial image. The container will have its name and hostname both set
to "mt1" (short for "mochi tutorial 1"). Note that the container is configured
to run indefinitely in detached mode and allow login for the "mochi" user with
no password.

The run command will also map the `mochi-tutorial` directory created in the
previous section to the `/home/mochi/mochi-tutorial` directory within the
container for convenience.  This will allow you to edit the code with your
editor of choice outside the container, and jump into the container to build
the code and run it (you can also choose to edit the code directory inside
the container using Vim if you prefer).

```
$ docker run --detach --name mt1 --volume $(pwd)/mochi-tutorial:/home/mochi/mochi-tutorial mochi-tutorial
```

Once the container is running, you can open a shell on it using the following
command.

```
$ docker exec -it mt1 /bin/bash
```

From the container's command prompt you should be in the `/home/mochi`
directory, with subdirectories available for `spack`,
`mochi-spack-packages`, and the `mochi-tutorial` directory that is mapped to
your host machine and populated with tutorial exercise files.

```
> docker exec -it mt1 /bin/bash

mochi@d3c9c489a2c1:~$ ls
mochi-spack-packages  mochi-tutorial  spack

mochi@d3c9c489a2c1:~$ ls mochi-tutorial
margo-tutorial-exercises
```

We recommand that you open multiple shells while following these exercises,
so that you can build the code, run a server and run a client in different
terminals

You can use the following commands to stop and restart the "mt1" container.

```
$ docker stop mt1
$ docker start mt1
```

If you need more detailed instructions or want to build the docker image
yourself from its Dockerfile recipe, please refer to
[these instructions](https://github.com/mochi-hpc-experiments/mochi-docker/).

### Option 2: create a development environment manually

Note that Spack setup and administration is beyond the scope of this
tutorial; please do not use this option unless you already have an existing
Spack configuration that you are comfortable using.

If you have Spack already installed and setup on your machine, simply make sure
that you have the Mochi namespace available for Spack to use. This can be done
as follows.

```
$ git clone https://github.com/mochi-hpc/mochi-spack-packages.git
$ spack repo add mochi-spack-packages
```

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

* You can test your client and server programs by opening two terminals
  (make sure you have run `spack env activate margo-tuto-env` in them to
  activate your spack environment) and running the following from the *build*
  directory.

  For the server:

```
$ src/server na+sm
```

  This will start the server and print its address. `na+sm` (the shared memory
  transport) may be changed to `tcp` if you run this code on multiple machines
  connected via an Ethernet network.

  For the client:

```
$ src/client na+sm <server-address>
```

  Copy `<server-address>` from the standard output of the server command.
  The server is setup to run indefinitely. You may kill it with Ctrl-C.
  Note that the server address will change whenever you restart it.

* Looking at the API in *phonebook.h*, edit *server.c* to add the creation
  of a phonebook object and its destruction when the server terminates.
  This phonebook should be added as a field to the `server_data` structure
  and to the `svr_data` instance (see comments `(1)` to `(3)` in *server.c*.
  This instance is attached to RPCs using `margo_register_data` so the
  phonebook can be accessed inside RPCs.

* Our two RPCs, which we will call "insert" and "lookup", will need
  argument and return types. Edit the *types.h* file to add the necessary
  type definitions for these RPCs (`insert_in_t`, `insert_out_t`, `lookup_in_t`
  and `lookup_out_t`, see comment `(4)`). Do so using the Mercury macros,
  following the model of the `sum_in_t` and `sum_out_t` types.
  _Hint: Mercury represents null-terminated strings with
  the type `hg_string_t`_, whose serialization routines are defined in
  the `mercury_proc_string.h` header.
  Note: while the insertion operation does not technically return anything,
  it is still advised to make all RPCs return at least a `uint32_t` error
  code to inform the sender of the success of the operation.

Note: if you only have half an hour to work on this problem, focus on
the _insert_ RPC and ignore the _lookup_ RPC. You can circle back to these
instructions for the latter if you have enough time.

* Edit *server.c* to add the definitions and declarations of the ULTs for
  our two RPCs (see comment `(5)` and `(6)`). Feel free to copy/paste and
  modify the existing `sum` RPC. Don't forget to register your RPCs with the
  margo instance in main (comment `(7)`), and don't forget to call
  `margo_register_data` to associate the server data with the RPC.

* Edit *client.c* and use the existing code as an example to (1) register
  the two RPCs here as well (comment `(8)`), and (2) define two `insert`
  and `lookup` functions that will take a `margo_instance_id` alongside the
  necessary arguments to create an `hg_handle_t`, forward it to the server
  with the proper arguments, and receive the response (comment `(9)`).
  These functions will need to call `margo_create` to create the `hg_handle_t`
  handle for the RPC, `margo_forward` to forward it to the server,
  `margo_get_output` to retrieve the response from the server,
  `margo_free_output` to free this response, and `margo_destroy` to destroy
  the `hg_handle_t` handle.

* Try out your code by calling `insert` and `lookup` a few times in `main`
  (comment `(10)`).

### Bonus: using RDMA to transfer larger amounts of data.

Do this bonus part only if you have time, or as an exercise later.
This part is less guided. You should now know how to add new RPCs to your code.

In this part, we will add a `lookup_multi` RPC that uses RDMA to send multiple
names at once and return the array of associated phone numbers (in practice
this would be too little data to call for the use of RDMA, but we will just
pretend). For this, you may use the example
[here](https://mochi.readthedocs.io/en/latest/margo/04_bulk.html).

Here are some tips for this part:

* On the client side, your `lookup_multi` function could take the number of
  names as a `uint32_t` and the list of names to look up as an array of
  null-terminated strings (`const char* const*`), as well as an output array
  of `uint64_t`. See comment `(11)` for a prototype.

* The important functions to work with RDMA are the following: `margo_bulk_create`
  (create an `hg_bulk_t` to expose a list of local memory segments for RDMA),
  `margo_bulk_transfer` (push/pull data to/from a local bulk handle, to/from
  a remote bulk handle), and `margo_bulk_free` (free a local `hg_bulk_t` created
  by `margo_bulk_create`). Alongside the documentation on ReadTheDocs, the
  [margo.h](https://github.com/mochi-hpc/mochi-margo/blob/main/include/margo.h)
  header provides the necessary information to work with these function.

* You will need to create two bulk handles on the client and two on the server.
  On the client, the first will expose the names as read-only (remember that
  `margo_bulk_create` can take a list of non-contiguous segments, but you will
  need to use `strlen(...)+1` as the size of each segment to keep the null
  terminator of each name), and the second will expose the output array as
  write only.

* You will need to transfer the two bulk handles in the RPC arguments,
  and since names can have a varying size, you will have to also transfer the
  total size of the bulk handle wrapping names, so that the server knows
  how much memory to allocate for its local buffer.  The `hg_bulk_t` type's
  serialization routines are defined in `mercury_proc_bulk.h`.

* On the server side, you will need to allocate two buffers; one to receive
  the names via a pull operation, the other to send the phone numbers via a
  push.

* You will need to create two `hg_bulk_t` to expose these buffers.

* After having transferred the names, they will be in the server's buffer, which,
  contrary to the client's memory, is contiguous. You can rely on the
  null-terminators to know where one name ends and the next starts.

## Exercise 2: a proper phonebook Mochi component

In this exercise, we will use the Margo microservice
[template](https://github.com/mochi-hpc/margo-microservice-template/) to
develop a proper phonebook microservice.

* Click on the green "Use this template" button and select
  "Create a new repository". Give the repository a name (e.g. "phonebook").
  Put the repository on "private" if you wish, then click on
  "Create repository from template".

* Click on Settings > Actions > General, and set the Workflow permissions to
  "Read and write permissions", then click 'Save' in the `Workflow Permissions`
  section (there are save buttons for each section that will save only the
  modifications made to that section)

* Go back to the root of the code (in your browser), and edit
  `initial-setup.json`. Change "alpha" to a service name of your choosing (this
  name will also be used to prefix your API functions, so choose something
  short, e.g. YP, for yellow pages. In the following, we will assume this is
  the name you used). Change "resource" to the name of the resource we are
  going to manage, here "phonebook". Click on the green "Commit changes"
  button.

* Wait a little and refresh the page. You might see a brown dot indicating a
  workflow action is in progress.  Or you might get a 404, which means the
  workflow completed: as part of the GitHub workflow that sets up your code,
  it will delete `initial-setup.json`.
  _Note: other github workflows will run to test your code and
  upload a coverage report to codecov.io whenever you push commits to GitHub.
  These workflows will not work properly if you have made the repository
  private, so you may receive emails from GitHub about some failed workflows.
  Simply ignore them._

* Clone your code in the docker container, then create a Spack environment
  and build the code like you did in Exercise 1, using the spack.yaml file at
  the root of your new project. Create a build directory and build the code.
  You may want to use the flag `-DENABLE_TESTS=ON` when calling `cmake` to make
  sure that the tests are also built.

It is now time to edit the code to make it do what we want. The template
provides the implementation of two RPCs, "hello" and "sum". You will recognize
the "sum" RPC as the same as provided in Exercise 1. This can be helpful to
reproduce what you have coded in Exercise 1 in the context of this new exercise.

* *include/YP/YP-client.h* contains the functions required to create and free
  a `YP_client` object, which will be used to register the RPCs and contact
  servers. There is nothing to modify in this file.

* *include/YP/YP-phonebook.h* declares a `YP_phonebook_handle_t` type, which
  represents a phonebook managed by a remote server. This file is also where
  the client interface will be defined. Go ahead and add declarations for
  a `YP_insert` and a `YP_lookup` functions, following what you did in Ex 1.

* *src/client.h* contains the definition of the `YP_client` structure.
  Go ahead and add two `hg_id_t` RPC ids to represent the insert and lookup.

* Before looking further into the client implementation, open *src/types.h*
  and add the type definitions for our RPCs (`insert_in_t`, `insert_out_t`,
  `lookup_in_t`, `lookup_out_t`, etc.). Take the example of the `sum_*`
  structures.

* *src/client.c* contains the implementation of the client-side functions.
  In `YP_client_init`, add the registration of our two new RPCs.

* Still in *src/client.c* and using code from Ex 1 (or by copying and
  adapting the content of the `YP_compute_sum` function), implement the
  client-side function that sends the `insert` and `lookup` RPCs to the server.

At this point, feel free to compile your code to make sure it builds fine
You won't be able to test it yet since there is no server-side implementation
of our RPCs, so let's focus on the server library next.

* *include/YP/YP-backend.h* contains the definition of a backend, i.e. the
  interface that a phonebook implementation must satisfy. Add the proper
  `insert` and `lookup` function pointers to this structure.

* *src/dummy/dummy-backend.c* contains a "dummy" implementation of such a
  backend. Copy the *phonebook.h* from the Ex 1 to the dummy folder, and
  include it in *dummy-backend.c*. Add a `phonebook_t phonebook` field to the
  `dummy_context` structure, then edit (1) `dummy_create_phonebook` and
  `dummy_open_phonebook` to add set this field (using `phonebook_new()`), and
  (2) `dummy_close_phonebook` and `dummy_destroy_phonebook` to call
  `phonebook_delete()`.

* Still in the same file, add the implementation of the `dummy_insert` and
  `dummy_lookup` functions and add the function pointers in the
  `static YP_backend_impl dummy_backend` structure definition.

* *src/provider.h* contains the state of a phonebook provider. Edit the
  *YP_provider* structure to add the `hg_id_t` for our two new RPCs, just like
  you did for the client.

* *src/provider.c* contains the implementation of the provider functions. Find
  the `YP_provider_register` function and add the registration of your new RPCs
  by taking the existing RPCs as examples. Such registration involves
  (1) calling `MARGO_REGISTER_PROVIDER` with the appropriate arguments,
  (2) calling `margo_register_data` to associate the provider with the RPC, and
  (3) setting the RPC ID in the provider structure.

* Still in *src/provider.c*, find the `YP_finalize_provider` function and
  add the calls necessary to deregister the two new RPCs. Note: you will first
  need to add declarations of your new RPCs, at the beginning of the file,
  where `DECLARE_MARGO_RPC_HANDLER` is used.

* We can now implement the functions that will handle the RPCs. In the same
  file, find the `YP_sum_ult` function, copy it (including the
  `DEFINE_MARGO_RPC_HANDLER` line that follows it) and edit it to transform it
  into a `YP_insert_ult` function, then do the same with a `YP_lookup_ult`
  function.

At this point, you can make sure your code builds fine. Your microservice is
ready! If you have time, feel free to look into the *tests* folder, in
particular the *test-client.c* file, and edit it (replacing calls to the "sum"
RPC) to try out your new functionalities.

In practice, the next steps at this point would be to (1) add more tests,
(2) remove everything related to the "hello" and "sum" RPCs (because obviously
a phonebook is not a calculator), and (3) implement more complex backends
by copying the code of the "dummy" backend and changing it to use external
libraries or more complicated implementations.

## Exercise 3: Using Bedrock and composing with other services

In this exercise we will use Bedrock to deploy a daemon containing
an instance of our phonebook provider. We will then implement a phonebook
backend that uses Yokan, and organize the composition of the two within
the same daemon. Everything in this exercise relies on the codebase from
Ex 2.

* First, make sure that the Spack environment from Ex 2 is activated.

* From the build directory, re-run cmake as follows.

```
$ cmake .. -DENABLE_TESTS=ON -DENABLE_BEDROCK=ON
$ make
```

This time a *libYP-bedrock-module.so* is being built. This is the Bedrock
module for our phonebook service, i.e. the library that tells Bedrock how
to instanciate and use our phonebook provider.

* To make sure Bedrock finds this library, execute the following command
  from the build directory.

```
$ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:`pwd`/src
```

* *examples/bedrock-config.json* is an example of Bedrock configuration that
  spins up a phonebook provider with provider ID 42. This provider manages
  one phonebook of type "dummy". You can try our this configuration using
  the `bedrock` program as follows.

```
$ bedrock na+sm -c ../examples/bedrock-config.json
```

* You can copy the address printed by bedrock on the last line of its log,
  and in another terminal (don't forget to activate your spack environment),
  run the following command.

```
$ bedrock-query na+sm -a <copied-address> -p
```

You will see the current configuration of the service, including a
phonebook provider that manages a phonebook. Bedrock has completed the
input configuration with a lot of information about Mercury, Argobots,
etc. These information can be very useful to communicate to Mochi developers
when you try to find out what's wrong with your service.

We will now add Yokan in our service.

* To add Yokan as dependency to our spack environment, run the following
  command.

```
$ spack add mochi-yokan+bedrock
$ spack install
```

This will install Yokan. You can also edit *spack.yaml* at the root of your
project to add `mochi-yokan+bedrock` in the list of specs.

* Edit *CMakeLists.txt* to add `find_package(yokan REQUIRED)` (e.g. after
  the call to `find_package(PkgConfig REQUIRED)`).
  _Note: when developing your own service, don't forget to also edit the
  `src/*.cmake.in` and `src/*.pc.in` files  to add relevant dependencies
  there. Those are the files used by cmake and pkg-config respectively to
  search for dependencies when people are using your code._

* Edit *src/CMakeLists.txt* to add yokan-client as a dependency for the
  YP-server library (i.e. find the call to `target_link_libraries` for
  YP-server and add yokan-client in the list of public dependencies).

* From the build directory, re-run `cmake ..` to make it find Yokan.

* Open *examples/bedrock-config.json* and add the Yokan library in the
  `libraries` section.

```json
"yokan": "libyokan-bedrock-module.so"
```

* In this file as well, we will instanciate a Yokan provider with
  a Yokan database. In the `providers` section, *before* the phonebook provider,
  add the following provider definition.

```json
{
  "type": "yokan",
  "name": "my-yokan-provider",
  "provider_id": 123,
  "config": {
    "databases": [
      {
        "type": "map",
        "name": "my-db"
      }
    ]
  }
},
```

If you re-run bedrock with this new configuration then call bedrock-query,
you should be able to confirm that your Bedrock daemon is now running two
providers: one YP provider and one Yokan provider. Of course, these two
don't know about each other, they simply share the resources of the same
process. We will now introduce a dependency between YP and Yokan.

* Edit *src/bedrock-module.c* and find the `struct bedrock_module`
  definition at the end. Its `provider_dependencies` field is where we
  will be able to introduce this dependency on Yokan. Before the declaration
  of the `bedrock_module` structure, add the following declaration:

```c
static struct bedrock_dependency provider_dependencies[] = {
    { "yokan_ph", "yokan", BEDROCK_REQUIRED },
    BEDROCK_NO_MORE_DEPENDENCIES
};
```

The first field, `"yokan_ph"`, is the name by which YP will reference
this dependency. `"yokan"` is the type of dependency. `BEDROCK_REQUIRED`
indicates that this dependency is required.

You can now assign the field in the `bedrock_module` structure.

```c
  .provider_dependencies = provider_dependencies
```

* If you rebuild your code now and re-run the Bedrock configuration,
  it will display an error message:

```
[critical] Missing dependency yokan_ph in configuration
```

So let's fix that by going again into *examples/bedrock-config.json*,
and add the following in the field in the definition of our YP provider.

```json
"dependencies": {
  "yokan_ph": "yokan:123@local"
}
```

You can also use `"my-yokan-provider"` instead of `"yokan:123"`.
Now Bedrock should restart accepting your configuration.

* In *include/YP/YP-server.h*, include the `yokan/provider-handle.h`
  header and add a `yokan_provider_handle_t yokan_ph` field in the
  `YP_provider_args structure`.

* Edit *src/bedrock-module.c* once again. This time we will look at the
  `YP_register_provider` function at the beginning of the file.
  Use `bedrock_args_get_dependency(args, "yokan_ph", 0);` to retrieve
  a pointer to a Yokan provider handle (`yoken_provider_handle_t`),
  which you can now use to set the corresponding field in the `YP_args`
  structure. You have successfully injected a Yokan dependency into
  the YP provider!

The rest of this exercise will be less directed. The goal is now to
pass this provider handle down to the dummy phonebook so that it can use
Yokan as an implementation of a key/value store instead of relying on
the *phonebook.h* implementation. You should now be familiar enough
with the code to make the necessary changes bellow without too much
guidance. Keep the API of Yokan open in a web browser for reference.
You can find it [here](https://github.com/mochi-hpc/mochi-yokan).

* To add and keep a reference to the Yokan provider handle in the `YP_provider`
  structure (in *src/provider.h*), you will need to copy the provided
  `yokan_provider_handle_t` in `YP_provider_register`, and free this copy in
  `YP_finalize_provider` (in *src/provider.c*). `yokan_provider_handle_t` is a
  public structure with no `yokan_provider_handle_ref_incr` function. You will
  have to manually copy its fields, and call `margo_addr_ref_incr` on the
  `hg_addr_t` field to increase the reference count of the address (and call
  `margo_addr_free` on it in `YP_finalize_provider`).

* To be able to pass the Yokan provider handle down to a backend (e.g. a dummy
  phonebook), you will need to change the signature of the functions that
  create and open a phonebook (in *include/YP/YP-backend.h*).

* This then implies changing their implementation (in
  *src/dummy/dummy-backend.c*).

* You will need to tell your dummy phonebook backend which database to use.
  Yokan databases can be identified by a name, so you may want to implement a
  way to look for the name of this database in the configuration passed to the
  phonebook (in the `dummy_create_phonebook` and `dummy_open_phonebook`
  functions in *src/dummy/dummy-backend.c*).

* Once a backend knows the name of the database it should use, you can use
  `yk_database_find_by_name` to look for its ID, then
  `yk_database_handle_create` to create a handle for the database (don't forget
  to call `yk_database_handle_release` when you no longer need it, e.g. when
  closing/destroying the dummy backend).

* In the `insert` and `lookup` functions of the dummy phonebook, you may now
  use `yk_put` and `yk_get` to put and get phone numbers.

In practice, you could copy the dummy backend implementation into a new
type of backend that specifically uses Yokan. Don't hesitate to implement
multiple backends for your service, with different dependencies or different
strategies for solving the same problem.
