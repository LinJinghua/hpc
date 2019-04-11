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

### Result
 - prerequisites
    - pip install redis
 - example
    ```
    import zlib
    s = b'''x\x00x'''
    print(s.hex())
    cs = zlib.compress(s)
    zlib.decompress(cs)

    import redis
    r = redis.Redis(host='localhost', port=6379, db=0)
    c = r.lrange('zinc_datazinc_ligand_1w_sort', 0, 0)[0]
    zlib.decompress(c)
    r.zadd('_zinc_datazinc_ligand_1w_sort', {'ZINC00717479' : 10})
    r.zcard('_zinc_datazinc_ligand_1w_sort')
    r.zrange('_zinc_datazinc_ligand_1w_sort', 0, -1, withscores=True)
    r.zscore('_zinc_datazinc_ligand_1w_sort', 'ZINC00717479')
    zlib.decompress(r.get('ZINC00717479'))

    # Get Result
    # see script/get_result.py
    ```

### Acknowledgements
This repository uses the following project(Alphabetical order) code:

 - [hiredis](https://github.com/redis/hiredis)
 - [mongo-c-driver](https://github.com/mongodb/mongo-c-driver)
 - [zlib](https://github.com/madler/zlib)

Special thanks to the contributors.

