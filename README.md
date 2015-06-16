# diceware
simple passphrase generation for linux

This uses the [diceware](http://world.std.com/~reinhold/diceware.html)
method to generate passphrases that are easy to remember while being
secure. It uses the original 7776 words that can also be uses to make
passwords using standard dice.

The default options will provide seven words, which should give about 90.5
bits of entropy. You can use the -n option to change the number of words.

In order for this to compile and work, you must have a recent linux
kernel, 3.17 or higher, that supports the getrandom(2) system call.
