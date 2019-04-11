import zlib
import redis
import pandas as pd

r = redis.Redis(host='localhost', port=6379, db=0)
key = '_zinc_datazinc_ligand_all'

encoding = 'utf-8'
num = 50 * 10000

zinc_id_list = []
score_list = []

for i in r.zrange(key, 0, num - 1, withscores=True):
    zinc_id = i[0].decode(encoding)
    score = i[1]
    data = zlib.decompress(r.get(zinc_id)).decode(encoding)
    # print(zinc_id, score, data)
    zinc_id_list.append(zinc_id)
    score_list.append(score)
    with open(zinc_id, "w") as mol_file:
        mol_file.write(data)

file_name = 'result.csv'
df = pd.DataFrame({'zinc_id': zinc_id_list, 'score': score_list})
df.to_csv(file_name, index=False, columns=['zinc_id', 'score'], sep='\t', encoding='utf-8')
