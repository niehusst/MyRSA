# MyRSA
An implementation of the RSA public key encryption algorithm. Just some
fun solving the key exchange problem.

### D&D - Dependencies and Demoing
This repository requires openSSL to be installed, and linked in such a
way that the compiler can find it (see this articles on [medium](https://medium.com/@timmykko/using-openssl-library-with-macos-sierra-7807cfd47892)).
I use the clang compiler in my Makefile, so make sure clang is installed as well.

All the files can be compiled by running `make` in the `src/` directory. This
will produce 2 executables; `client` and `server` that you can use to run simple
TCP echo server. You can either run each program in their own terminal window,
or you can run `server` as a background process; `./server &`. Notice it prints
out a port number. Remember that now. (Note that if you choose to run `server`
as a background process, it won't terminate on its own; you can kill the process
with `kill <server pid>`.)

Then, run the `client` program, via `./client localhost <PORT_NUMBER>`. Unless
you have another computer you want to run this program on, you will be connecting
the `client` and `server` over the host localhost with the port number printed by `server`.
Finally you can use it! With `client`, you can enter a message that will be encrypted
and sent over the network to `server`, who will decrypt it, read it, re-encrypt
it using its own RSA key and send it back to you (because it has nothing better
to say) where `client` will decrypt the message and display it to the terminal.

## Authors
* **Liam Niehus-Staab** - [niehusst](https://github.com/niehusst)

## Acknowledgements
* "History of Cryptography and Cryptanalysis" by John F. Dooley for inspiration and RSA algorithm details.
* Wikipedia articles for explaining some of the math contained in the RSA algorithm
* openSSL for its extremely fast large-prime-number generation and math functions provided under Apache license
