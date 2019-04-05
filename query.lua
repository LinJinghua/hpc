-- SCHE=$(cat ./sehe.host)
-- ./redis-cli -h ${SCHE} --eval query.lua

local unfinished = redis.call('LLEN', 'zinc_datazinc_ligand_all')
local finished = redis.call('ZCARD', '_zinc_datazinc_ligand_all')

return {"unfinished: "..unfinished, "finished: "..finished};
