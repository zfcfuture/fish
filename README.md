# Unix-like shell: fish

We inroduced a simple but powerful user-built unix-like shell in this repository.

## Environment 

This project based on unix-liked operation system, and we choose `C` language as the programming language. So you need to install the `GCC` related libraries. 

You can check your environment by this command:

```shell
gcc -v
```

And then, you will get the version number of `gcc`, if not, try to install `gcc` and `make` by this command:

```shell
#for centos
sudo yum -y install gcc  
sudo yum -y install gcc-c++ 
sudo yum -y install make

#for ubuntu
sudo apt-get install gcc
sudo apt-get install g++
sudo apt-get install make
```

## Execute

Just run the command `make`, you will get the ELF file called `fish`, try to execute it by this command:

```shell
make
./fish
```

At this point, just enjoy it.

Note: `fish`(Version 0.1) only support normal, input redirect, output redirect and pipe command   : )
