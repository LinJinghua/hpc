# High-throughput system for material calculation

### Prerequisites
 - [reids service](https://github.com/antirez/redis)
 - [schedule service](https://github.com/LinJinghua/schedule)


### Build & Run
```shell
mkdir -p build && cd build
cmake ..
make

./producer <reids server hostname> <reids server port> <mongodb server url>
./consumer <schedule hostname:schedule port> <redis queue key>
```

### Acknowledgements
This repository uses the following project(Alphabetical order) code:
 - [hiredis](https://github.com/redis/hiredis)
 - [mongo-c-driver](https://github.com/mongodb/mongo-c-driver)
 - [zlib](https://github.com/madler/zlib)

Special thanks to the contributors.

