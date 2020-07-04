# NF-TinyACL
NTL, which stands for NF-TinyACL is a netfilter based ACL kernel module for studying. With this kernel module, administrator can control incoming packets which their eth source address are the same as specific mac address in **Deny List** at bridge pre routing time.

*The purpose of this module is studying for kernel network subsystem. No warranty on any risky situation and you have to verify all together before put in release. I have no responsibility for that case at all.*

## Module Workflow
![diagram](/resources/diagram/diagram.png)

## Commands
##### Add
```sh
$ echo "add XX:XX:XX:XX:XX:XX" > /sys/kernel/debug/ntl/bridge/deny_list
```
##### Delete
```sh
$ echo "del XX:XX:XX:XX:XX:XX" > /sys/kernel/debug/ntl/bridge/deny_list
```
##### Show
```sh
$ cat /sys/kernel/debug/ntl/bridge/deny_list
```

## License
Dual BSD/GPL

## Thanks to
- *Christian Benvenuti*, who wrote such a great book, **Understanding Linux Network Internals**.
- *문c*, who wrote great articles about Linux Kernel features, especially **RCU** part was helpful.
- *Austin Kim*, who wrote a great book, **디버깅을 통해 배우는 리눅스 커널의 구조와 원리**.
- *Kernel Hackers*, who wrote such great source code.
