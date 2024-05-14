require('dotenv').config();
const {gql, request} = require('graphql-request');
const addon = require('bindings')('sssp');

const init = async () => {
    const startApi = performance.now();
    const uni3Url = `https://gateway.thegraph.com/api/${process.env.API_KEY}/subgraphs/id/EN9rjKtzNitTEb5hgt8bmiyzzhwBpJrJaRihkg8Me8Rr`;
    const sushiUrl = `https://gateway.thegraph.com/api/${process.env.API_KEY}/subgraphs/id/D7azkFFPFT5H8i32ApXLr34UQyBfxDAfKoCEK4M832M6`;
    const uni2Url = `https://gateway.thegraph.com/api/${process.env.API_KEY}/subgraphs/id/2szAn45skWZFLPUbxFEtjiEzT1FMW8Ff5ReUPbZbQxtt`;
    const [uni, sushi, uni2] = await Promise.all([
        importUniswap3Pools(uni3Url),
        importSushiswapPools(sushiUrl),
        importUniswap2Pools(uni2Url)
    ]);
    const apiDuration = performance.now() - startApi;
    console.log('Api Retrieve performance:', apiDuration);

    const sPopulateCpp = performance.now();
    uni2.pairs?.forEach(data => {
        addCppEntry(data.token0.id, data.token1.id, data.token0Price, data.token1Price, data.id);
    });
    sushi.pairDayDatas?.forEach(data => {
        addCppEntry(data.pair.token0.id, data.pair.token1.id, data.pair.token0Price, data.pair.token1Price, data.pair.id);
    });
    uni.poolDayDatas?.forEach(data => {
        addCppEntry(data.pool.token0.id, data.pool.token1.id, data.pool.token0Price, data.pool.token1Price, data.pool.id);
    });

    addon.sssp();
    const dPopulateCpp = performance.now() - sPopulateCpp;
    console.log('Populate CPP performance:', dPopulateCpp);

}

const addCppEntry =  (t1, t2, tp1, tp2, pool) => {
    addon.addPool(t1, tp1, t2, tp2, pool);
}

const importSushiswapPools = async (dexUrl) => {
    const currentDate = new Date();
    currentDate.setDate(currentDate.getDate() - 2);
    currentDate.setHours(3,0,0,0);

    const gqlRequest = gql `
    {
      pairDayDatas(first: 1000, orderBy: volumeUSD, orderDirection: desc, where:{date_gte:${currentDate.getTime()/1000}, reserveUSD_gte:200000}) {
        id
        date
        pair {
          id
          token0 {
            id
            symbol
            name
          }
          token0Price
          token1 {
            id
            symbol
            name
          }
          token1Price
        }
      }
    }
    `;

    const sushi = await request(dexUrl, gqlRequest);

    const sushipools = [];
    for (let i = sushi.pairDayDatas.length - 1; i >= 0; i--) {
        if (sushipools.indexOf(sushi.pairDayDatas[i].pair.id) > 0) {
            delete sushi.pairDayDatas[i];
        } else {
            sushipools.push(sushi.pairDayDatas[i].pair.id);
        }
    }

    return sushi;
}

const importUniswap2Pools = async (dexUrl) => {
    const currentDate = new Date();
    currentDate.setDate(currentDate.getDate() - 2);
    currentDate.setHours(3,0,0,0);

    const gqlRequest1 = gql `
    {
      pairDayDatas(first: 1000, orderBy: dailyVolumeUSD, orderDirection:desc, where:{date_gte:${currentDate.getTime()/1000}, reserveUSD_gte:200000}) {
        date
        pairAddress
      }
    }
    `;

    const uni1 = await request(dexUrl, gqlRequest1);

    const unipools = [];
    for (let i = uni1.pairDayDatas.length - 1; i >= 0; i--) {
        if (unipools.indexOf(uni1.pairDayDatas[i].pairAddress) > 0) {
            delete uni1.pairDayDatas[i];
        } else {
            unipools.push(uni1.pairDayDatas[i].pairAddress);
        }
    }

    let idString = '[\"' + unipools.join("\",\"") + "\"]";
    const gqlRequest2 = gql `
     {
        pairs (where: {id_in: ${idString}, reserveUSD_gte: 200000}) {
            id
            token0 {
                id
                symbol
                name
            }
            token0Price
            token1 {
                id
                symbol
                name
            }
            token1Price
        }
     }
    `;

    return request(dexUrl, gqlRequest2);
}

const importUniswap3Pools = async (dexUrl) => {
    const currentDate = new Date();
    currentDate.setDate(currentDate.getDate() - 2);
    currentDate.setHours(3,0,0,0);

    const gqlRequest = gql `
    {
      poolDayDatas(first: 1000, orderBy:volumeUSD, orderDirection: desc, where:{date_gte:${currentDate.getTime()/1000}, volumeUSD_gte:200000}) {
        volumeUSD
        date
        pool{
          id
          feeTier
          token0{
            symbol
            id
            name
          }
          token0Price
          token1{
            symbol
            id
            name
          }
          token1Price
        }
      }
    }
    `;

    const uni = await request(dexUrl, gqlRequest);

    const unipools = [];
    for (let i = uni.poolDayDatas.length - 1; i >= 0; i--) {
        if (unipools.indexOf(uni.poolDayDatas[i].pool.id) > 0 || uni.poolDayDatas[i].pool.token0.symbol === 'UST' || uni.poolDayDatas[i].pool.token1.symbol === 'UST') {
            delete uni.poolDayDatas[i];
        } else {
            unipools.push(uni.poolDayDatas[i].pool.id);
        }
    }

    return uni;
}

init();