local W = 256
local H = 256

function generate_heightmap(x, y)
    local umap = Heightmap(W, H)
    local vmap = Heightmap(W, H)
    umap:noise({x+521, y+73}, 0.05, 1, 20.8)
    umap:noise({x+51, y+75}, 0.05, 1, 21.8)
    umap:noise({x+521, y+70}, 0.1, 3, 35.8)
    vmap:noise({x+95, y+246}, 0.15, 3, 35.8)

    local bmap = Heightmap(W, H)
    bmap:noise({x+3, y+6}, 0.1, 1, 3)
    
    local map = Heightmap(W, H)
    map:noise({x, y}, 0.06, 5, 0.2, umap, vmap)
    map:noise({x, y}, 0.12, 6, 0.5, umap, vmap)
    map:mul(bmap)
    map:mul(0.7)

    local rivermap = Heightmap(W, H)
    rivermap:noise({x+21, y+12}, 0.1, 3)
    rivermap:abs()
    rivermap:min(0.5)
    rivermap:mul(2.0)
    rivermap:pow(0.6)
    map:add(1.2)
    map:mul(rivermap)
    map:add(-1.0)
    map:mul(0.5)

    return map
end

local map = generate_heightmap(0, 0)
map:dump("heightmap.png")
