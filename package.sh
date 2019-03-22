#! /bin/bash
cmake ..
make
mkdir -p release
cp -vp ./producer ../lib/*.so* release/
tar -zcf release.tar.gz release/

# Examples:
# ./producer 127.0.0.1 6379 "mongodb://127.0.0.1:27017" zinc_data zinc_ligand_1w_sort
# ./producer 10.186.5.116 6379 "mongodb://12.11.70.12:10101" wega_data DrugBank
# ./producer 10.186.5.116 6379 "mongodb://12.11.70.12:10101" wega_data SPECS
# ./producer 10.186.5.116 6379 "mongodb://12.11.70.12:10101" wega_data SYSU_SMR
# ./producer 10.186.5.116 6379 "mongodb://12.11.70.12:10101" wega_data ZINC
