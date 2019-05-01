import zlib
import redis

r = redis.Redis(host='localhost', port=6379, db=0)
key = '_xy'

encoding = 'utf-8'
num = 0.5 * 10000
for i in range(int(num)):
    zinc_id = i
    score = i
    data = zlib.compress(bytes(str(i), encoding=encoding))
    r.zadd(key, {zinc_id: score})
    r.set(zinc_id, data)
