-- SCHE=$(cat ./sehe.host)
-- ./redis-cli -h ${SCHE} --eval query.lua

local unfinished = redis.call('LLEN', 'zinc_datazinc_ligand_all')
local finished = redis.call('ZCARD', '_zinc_datazinc_ligand_all')
local unfinished_ = redis.call('LLEN', 'zinc_datazinc_ligand_1w_sort')
local finished_ = redis.call('ZCARD', '_zinc_datazinc_ligand_1w_sort')

return {"unfinished: "..unfinished, "finished: "..finished, "1w u:"..unfinished_, "1w f:"..finished_};
