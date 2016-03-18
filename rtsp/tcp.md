# TCP

## Sever
### [Socket](http://man7.org/linux/man-pages/man2/socket.2.html)
**protocol family**: _AF_INET_(IPv4)  
**type**: _SOCK_STREAM_(TCP)  
**protocol** Normally only a single protocol exists to support a particular socket type within a given protocol family, in which case protocol can be specified as _0_.  
**RETURN VALUE**: a file descriptor on success; -1 on error.

### Address
[struct sockaddr_in](http://man7.org/linux/man-pages/man7/ip.7.html) (IPv4 protocol)  
* **sin_family**: _AF_INET_  
* **sin_port**: port in network byte order(See [htons](http://man7.org/linux/man-pages/man3/htons.3.html)).  
* **sin_addr**:
  * **s_addr**: address in network byte order, usually set to INADDR_ANY.  

### [Bind](http://man7.org/linux/man-pages/man2/bind.2.html)
Assigns the specified address to the socket.  
**sockfd**(int): fd given by socket().  
**addr**(struct sockaddr*): casted from relative _Address_.  
**addrlen**(socklen_t): sizeof addr struct.  
**RETURN VALUE**: 0 on success; -1 on error.

### [Listen](http://man7.org/linux/man-pages/man2/listen.2.html)
Marks the socket as a passive socket.  
**sockfd**(int)  
**backlog**(int): the maximum length to which the queue of pending connections for sockfd may grow.  
**RETURN VALUE**: 0 on success; -1 on error.

### [Accept](http://man7.org/linux/man-pages/man2/accept.2.html)
Extracts the first connection request on the queue of pending connections for the listening socket, and creates a new connected socket.  
**sockfd**(int)  
**addr**(struct sockaddr\*)  
**addrlen**(socklen_t\*)  
**RETURN VALUE**: a file descriptor for the new connected socket on success; -1 on error.

### [Recv](http://man7.org/linux/man-pages/man2/recvmsg.2.html)
Receive messages from a socket.  
**sockfd**(int)  
**buf**(void\*)  
**len**(size_t)  
**flags**(int)  
**RETURN VALUE**: the number of bytes received; -1 on error.

### [Send](http://man7.org/linux/man-pages/man2/sendmsg.2.html)
Transmit a message to another socket.  
**sockfd**(int)  
**buf**(void\*)  
**len**(size_t)  
**flags**(int)  
**RETURN VALUE**: the number of bytes sent; -1 on error.

### [Close](http://man7.org/linux/man-pages/man2/close.2.html)

## Client
### Socket

### Address
>C  
struct hostent *gethostbyname(const char *name);  
struct hostent {  
   char  *h_name;            /* official name of host */  
   char **h_aliases;         /* alias list */  
   int    h_addrtype;        /* host address type */  
   int    h_length;          /* length of address */  
   char **h_addr_list;       /* list of addresses */  
   char  *h_addr;            /* first address in h_addr_list */  
}

### [Connect](http://man7.org/linux/man-pages/man2/connect.2.html)
Connects the server socket.  
**sockfd**(int): fd given by socket().  
**addr**(struct sockaddr\*): casted from relative _Address_.  
**addrlen**(socklen_t): sizeof addr struct  
**RETURN VALUE**: 0 on success; -1 on error.

### Send

### Recv

### Close