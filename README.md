  The dnsproxy daemon is a proxy for DNS queries. It forwards these queries
  to two previously configured nameservers: one for authoritative queries
  and another for recursive queries. The received answers are sent back to
  the client unchanged. No local caching is done.

  Primary motivation for this project was the need to replace Bind servers
  with djbdns in an ISP environment. These servers get recursive queries
  from customers and authoritative queries from outside at the same IP
  address. Now it is possible to run dnscache and tinydns on the same
  machine with queries dispatched by dnsproxy.

  Other scenarios are firewalls where you want to proxy queries to the real
  servers in your DMZ. Or internal nameservers behind firewalls, or...

  Some features:
  - Secure. Runs chrooted and without root privileges.
  - Fast. Able to process hundreds of queries per second.
  - Easy to setup. Simple configuration file.

#### INSTALLATION

  If you run any current UNIX-like operating system, only the following
  steps should be necessary:
  ```
    ./configure
    make
    make install
  ```
  Unlike previous releases the @PACKAGE_VERSION@ version does not include the libevent
  library. It should now also be available from your operating systems
  ports/packages/rpm/deb repository or as always from the source at
  http://www.monkey.org/~provos/libevent/. If it is not found by the
  configure script you may specify the directory prefix of your libevent
  installation with: 
  ```
    ./configure --with-libevent=/usr/local
  ```

#### CONFIGURATION

  At startup dnsproxy reads a configuration file specified via the -c
  option or at the default location of /etc/dnsproxy.conf. The syntax of
  this configuration file is of the form "keyword value" and looks like:
  ```
    authoritative         127.0.0.1   # Authoritative server. Required.
    authoritative-port    53001       # It's port. Defaults to 53.
    authoritative-timeout 10          # Seconds to wait for answers.

    recursive         127.0.0.1       # Recursive server. Required.
    recursive-port    53002           # It's port. Defaults to 53.
    recursive-timeout 90              # Seconds to wait for answers.

    listen 192.168.168.1              # Dnsproxy's listen address.
    port   53                         # Dnsproxy's port address.

    chroot /var/empty                 # Directory to chroot dnsproxy.
    user   nobody                     # Unprivileged user to run as.

    internal 192.168.168.0/24         # Only internal IP addresses are
    internal 127.0.0.1                # allowed to do recursive queries.

    statistics 3600                   # Print statistics every hour.
  ```

#### COMPATIBILITY

  This package should build and run on OpenBSD, FreeBSD, Solaris, Linux.
  Other POSIX environments should work also. Please drop me a mail if
  you run dnsproxy on an unlisted system.

#### LICENSE

  This software is released under an OSI approved MIT-style license.

