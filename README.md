# traceroute
It is a project made for the subject of computer networks. It is utilizing the ICMP protocol.
## Usage
```
$ make
$ sudo ./traceroute A.B.C.D
```
where ```A.B.C.D ``` is the desired IPv4 address.

If you want to get rid of the ```.o``` files afterwards use the ```make clean```. In case you wanted to delete the binary as well, ```make distclean``` will leave only the source files.
