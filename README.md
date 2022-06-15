# Character device
This project consist in a character device /dev/hello that support reading and writing operations.

## Example of usage
To write you can simply type
```
$ echo "Hello!" > /dev/hello
```

To read:
```
$ cat /dev/hello

Hello!
```

## Pre-requisites
To compile the driver under linux you have make sure that the linux-headers are installed. 
If not you can simply install using 
```
apt install linux-headers
```
## Compile and load
```
make && sudo insmod hello.ko
```

## Unload
If you want to unload the drvier use

```
sudo rmmod hello.ko
```

## Clean Binary
If you want clean .o and .ko files

```
make clean
```