# MyRSA
An implementation of the RSA public key encryption algorithm. Just some
fun solving the key exchange problem.

### How it all works

RSA encryption is used for sending encrypted messages to a target destination without having to somehow send over a key to decrypt the messages as well. Everyone who participates in secure communication using RSA needs two encryption keys: a public key and a private key. 

A breif description of what the keys are and how they work:
The public key is made up of a a few numbers. The first is a number `n` that is the product of two VERY large prime numbers `p` and `q` (i.e. 256 bits or greater depending on how much security you want), and the second is a number `e` that is co-prime to the Euler totient of `p` and `q`. Together, `n` and `e` make up the public key. The private key is comprised of another two numbers; `n` again and the multiplicative inverse `i` of the previously calculated totient. These specific numbers are chosen because of the useful property that a number can be "encrypted" by taking its modular exponent with `n` and `e` ```(number ^ e) mod n```
and then that same number can be recovered, or "decrypted", by taking another modular exponent with `n` and `i`!
```(encrypted_number ^ i) mod n```
And primes are used because they allow this property and they are large to decrease the chances of a hacker cracking the private key. (For more information on this process and the math, checkout [wikipedia](https://en.wikipedia.org/wiki/RSA_(cryptosystem)): the source of all knowledge.)

Now, all you need to do in order to send a message encrypted such that only the intended recipient can read it is exchange public keys with them! You encrypt your messages to them using their public key, which they can then decipher using their own private key. Since you don't want anyone else to be able to read messages intended for only you, the private key should never be shared.

This is great! We can now send and receive messages very securely ... if they are numbers. The process of converting text to numbers is called padding. The most important part of a padding algorithm is that any input text can be uniquely converted to a number and back to the same text with no collisions (a bijective function for you fancy people out there). There are many ways of doing this, but I use a very simple one since this isn't a production environment; I simply convert each character to its ascii value and concatenate that number (in a chunk of 3 digits) onto the back of the resulting number. To unpad, you simply parse each block of 3 digits to ascii again.

The final part of this project is just a TCP/IP echo server using the C sockets API so there is anyone to send messages to without having to go through the trouble of somehow integrating this code into another communication system just to test it out.

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
