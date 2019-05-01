import zlib
import redis
import pandas as pd
from pymongo import MongoClient

import sys
import getopt

encoding = 'utf-8'

def get_zinc_bson(zinc_entry, data):
    zinc_id = zinc_entry[0]
    score = zinc_entry[1]
    return {
        '_id': zinc_id.decode(encoding),
        'score': score,
        # 'data': zlib.decompress(data).decode(encoding)
        'data': data
    }

def process(collection, redis_client, key):
    num = 10000
    try:
        succ = 0
        while True:
            li = redis_client.zrange(key, 0, num - 1, withscores=True)
            if len(li) == 0:
                break
            for i in li:
                collection.insert_one(get_zinc_bson(i, redis_client.get(i[0])))
                redis_client.zrem(key, i[0])
                redis_client.delete(i[0])
                succ = succ + 1
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
    return '_' + (db_name + collection_name).replace(' ', '')

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
''')


def main(argv=None):
    if argv is None:
        argv = sys.argv

    mongo_url = 'mongodb://localhost:27017/'
    db_name = 'zinc_data'
    collection_name = 'zinc_ligand_1w_sort'
    redis_host = 'localhost'
    redis_port = 6379

    try:
        options, args = getopt.getopt(sys.argv[1:],
            "hu:d:c:r:p:", ['help', 'url=', 'database=', 'collection=', 'redis=', 'port='])
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
            else:
                print("unhandled option")
                return -1
        process(get_collection(mongo_url, db_name, collection_name),
            get_redis(redis_host, redis_port),
            get_redis_key(db_name, collection_name))

    except getopt.GetoptError:
        usage()
        return 2


if __name__ == "__main__":
    sys.exit(main())

