import zlib
import redis
import pandas as pd
from pymongo import MongoClient

import sys
import getopt

encoding = 'utf-8'

def read_datas(path):
    try:
        return pd.read_csv(path)
    except IOError as err:
        print("I/O error: {0}".format(err))
        raise
    except:
        print("Unexpected error:", sys.exc_info()[0])
        raise
    return None

def compress(data):
    return zlib.compress(bytes(data, encoding=encoding))

def get_file(filename):
    with open(filename) as f:
        return f.read()

def get_entry(name, data):
    return name + '\0' + data

def process(collection, redis_client, key, datas):
    name_str = 'name'
    data_str = 'data'
    try:
        succ = 0
        for id_str, path in zip(datas[name_str], datas[data_str]):
            data = get_file(path)
            collection.insert_one(
                {name_str: id_str, data_str: compress(data)})
            succ = succ + 1
            redis_client.lpush(key, compress(get_entry(id_str, data)))
    except:
        print("Unexpected error:", sys.exc_info()[0])
        # raise
    print('succ:', succ)
    return None

def get_collection(url, db_name, collection_name):
    try:
        # db.list_collection_names()
        return MongoClient(url)[db_name][collection_name]
    except:
        print("Unexpected error:", sys.exc_info()[0])
        raise

def get_redis(host, port):
    return redis.Redis(host=host, port=port)

def get_redis_key(db_name, collection_name):
    return (db_name + collection_name).replace(' ', '')

def usage():
    print('''pmongo version v0.0.1
usage: pmongo [options]
Options:
    -h help
    -u mongo url["mongodb://localhost:27017/"]
    -d database name["zinc_data"]
    -c collection name["zinc_ligand_1w_sort"]
    -r redis host["localhost"]
    -p csv file["6379"]
    -f csv file["submit.csv"]
''')


def main(argv=None):
    if argv is None:
        argv = sys.argv

    mongo_url = 'mongodb://localhost:27017/'
    db_name = 'zinc_data'
    collection_name = 'zinc_ligand_1w_sort'
    redis_host = 'localhost'
    redis_port = 6379
    csv_file = 'submit.csv'

    try:
        options, args = getopt.getopt(sys.argv[1:],
            "hu:d:c:r:p:f:", ['help', 'url=', 'database=', 'collection=', 'redis=', 'port=', 'file='])
        # print(options, args)

        for name, value in options:
            if name in ('-h', '--help'):
                usage()
                return 0
            elif name in ('-u', '--url'):
                mongo_url = value
            elif name in ('-d', '--database'):
                db_name = value
            elif name in ('-c', '--collection'):
                collection_name = value
            elif name in ('-r', '--redis'):
                redis_host = value
            elif name in ('-p', '--port'):
                redis_port = int(value)
            elif name in ('-f', '--file'):
                csv_file = value
            else:
                print("unhandled option")
                return -1
        process(get_collection(mongo_url, db_name, collection_name),
            get_redis(redis_host, redis_port),
            get_redis_key(db_name, collection_name),
            read_datas(csv_file))

    except getopt.GetoptError:
        usage()
        return 2


if __name__ == "__main__":
    sys.exit(main())

