/**
 * Copyright 2021 Ethan.cr.yp.to
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mpi.h"
#include <mpn/mpn-asm.h>
#include <mpn/mpn-binary.h>
#include <mpn/mpn-montgomery.h>

// clang-format off
static const uint32_t known_primes[] = {
    2,     3,     5,     7,     11,    13,    17,    19,    23,    29,    31,    37,    41,    43,    47,    53,
    59,    61,    67,    71,    73,    79,    83,    89,    97,    101,   103,   107,   109,   113,   127,   131,
    137,   139,   149,   151,   157,   163,   167,   173,   179,   181,   191,   193,   197,   199,   211,   223,
    227,   229,   233,   239,   241,   251,   257,   263,   269,   271,   277,   281,   283,   293,   307,   311,
    313,   317,   331,   337,   347,   349,   353,   359,   367,   373,   379,   383,   389,   397,   401,   409,
    419,   421,   431,   433,   439,   443,   449,   457,   461,   463,   467,   479,   487,   491,   499,   503,
    509,   521,   523,   541,   547,   557,   563,   569,   571,   577,   587,   593,   599,   601,   607,   613,
    617,   619,   631,   641,   643,   647,   653,   659,   661,   673,   677,   683,   691,   701,   709,   719,
    727,   733,   739,   743,   751,   757,   761,   769,   773,   787,   797,   809,   811,   821,   823,   827,
    829,   839,   853,   857,   859,   863,   877,   881,   883,   887,   907,   911,   919,   929,   937,   941,
    947,   953,   967,   971,   977,   983,   991,   997,   1009,  1013,  1019,  1021,  1031,  1033,  1039,  1049,
    1051,  1061,  1063,  1069,  1087,  1091,  1093,  1097,  1103,  1109,  1117,  1123,  1129,  1151,  1153,  1163,
    1171,  1181,  1187,  1193,  1201,  1213,  1217,  1223,  1229,  1231,  1237,  1249,  1259,  1277,  1279,  1283,
    1289,  1291,  1297,  1301,  1303,  1307,  1319,  1321,  1327,  1361,  1367,  1373,  1381,  1399,  1409,  1423,
    1427,  1429,  1433,  1439,  1447,  1451,  1453,  1459,  1471,  1481,  1483,  1487,  1489,  1493,  1499,  1511,
    1523,  1531,  1543,  1549,  1553,  1559,  1567,  1571,  1579,  1583,  1597,  1601,  1607,  1609,  1613,  1619,
    1621,  1627,  1637,  1657,  1663,  1667,  1669,  1693,  1697,  1699,  1709,  1721,  1723,  1733,  1741,  1747,
    1753,  1759,  1777,  1783,  1787,  1789,  1801,  1811,  1823,  1831,  1847,  1861,  1867,  1871,  1873,  1877,
    1879,  1889,  1901,  1907,  1913,  1931,  1933,  1949,  1951,  1973,  1979,  1987,  1993,  1997,  1999,  2003,
    2011,  2017,  2027,  2029,  2039,  2053,  2063,  2069,  2081,  2083,  2087,  2089,  2099,  2111,  2113,  2129,
    2131,  2137,  2141,  2143,  2153,  2161,  2179,  2203,  2207,  2213,  2221,  2237,  2239,  2243,  2251,  2267,
    2269,  2273,  2281,  2287,  2293,  2297,  2309,  2311,  2333,  2339,  2341,  2347,  2351,  2357,  2371,  2377,
    2381,  2383,  2389,  2393,  2399,  2411,  2417,  2423,  2437,  2441,  2447,  2459,  2467,  2473,  2477,  2503,
    2521,  2531,  2539,  2543,  2549,  2551,  2557,  2579,  2591,  2593,  2609,  2617,  2621,  2633,  2647,  2657,
    2659,  2663,  2671,  2677,  2683,  2687,  2689,  2693,  2699,  2707,  2711,  2713,  2719,  2729,  2731,  2741,
    2749,  2753,  2767,  2777,  2789,  2791,  2797,  2801,  2803,  2819,  2833,  2837,  2843,  2851,  2857,  2861,
    2879,  2887,  2897,  2903,  2909,  2917,  2927,  2939,  2953,  2957,  2963,  2969,  2971,  2999,  3001,  3011,
    3019,  3023,  3037,  3041,  3049,  3061,  3067,  3079,  3083,  3089,  3109,  3119,  3121,  3137,  3163,  3167,
    3169,  3181,  3187,  3191,  3203,  3209,  3217,  3221,  3229,  3251,  3253,  3257,  3259,  3271,  3299,  3301,
    3307,  3313,  3319,  3323,  3329,  3331,  3343,  3347,  3359,  3361,  3371,  3373,  3389,  3391,  3407,  3413,
    3433,  3449,  3457,  3461,  3463,  3467,  3469,  3491,  3499,  3511,  3517,  3527,  3529,  3533,  3539,  3541,
    3547,  3557,  3559,  3571,  3581,  3583,  3593,  3607,  3613,  3617,  3623,  3631,  3637,  3643,  3659,  3671,
    3673,  3677,  3691,  3697,  3701,  3709,  3719,  3727,  3733,  3739,  3761,  3767,  3769,  3779,  3793,  3797,
    3803,  3821,  3823,  3833,  3847,  3851,  3853,  3863,  3877,  3881,  3889,  3907,  3911,  3917,  3919,  3923,
    3929,  3931,  3943,  3947,  3967,  3989,  4001,  4003,  4007,  4013,  4019,  4021,  4027,  4049,  4051,  4057,
    4073,  4079,  4091,  4093,  4099,  4111,  4127,  4129,  4133,  4139,  4153,  4157,  4159,  4177,  4201,  4211,
    4217,  4219,  4229,  4231,  4241,  4243,  4253,  4259,  4261,  4271,  4273,  4283,  4289,  4297,  4327,  4337,
    4339,  4349,  4357,  4363,  4373,  4391,  4397,  4409,  4421,  4423,  4441,  4447,  4451,  4457,  4463,  4481,
    4483,  4493,  4507,  4513,  4517,  4519,  4523,  4547,  4549,  4561,  4567,  4583,  4591,  4597,  4603,  4621,
    4637,  4639,  4643,  4649,  4651,  4657,  4663,  4673,  4679,  4691,  4703,  4721,  4723,  4729,  4733,  4751,
    4759,  4783,  4787,  4789,  4793,  4799,  4801,  4813,  4817,  4831,  4861,  4871,  4877,  4889,  4903,  4909,
    4919,  4931,  4933,  4937,  4943,  4951,  4957,  4967,  4969,  4973,  4987,  4993,  4999,  5003,  5009,  5011,
    5021,  5023,  5039,  5051,  5059,  5077,  5081,  5087,  5099,  5101,  5107,  5113,  5119,  5147,  5153,  5167,
    5171,  5179,  5189,  5197,  5209,  5227,  5231,  5233,  5237,  5261,  5273,  5279,  5281,  5297,  5303,  5309,
    5323,  5333,  5347,  5351,  5381,  5387,  5393,  5399,  5407,  5413,  5417,  5419,  5431,  5437,  5441,  5443,
    5449,  5471,  5477,  5479,  5483,  5501,  5503,  5507,  5519,  5521,  5527,  5531,  5557,  5563,  5569,  5573,
    5581,  5591,  5623,  5639,  5641,  5647,  5651,  5653,  5657,  5659,  5669,  5683,  5689,  5693,  5701,  5711,
    5717,  5737,  5741,  5743,  5749,  5779,  5783,  5791,  5801,  5807,  5813,  5821,  5827,  5839,  5843,  5849,
    5851,  5857,  5861,  5867,  5869,  5879,  5881,  5897,  5903,  5923,  5927,  5939,  5953,  5981,  5987,  6007,
    6011,  6029,  6037,  6043,  6047,  6053,  6067,  6073,  6079,  6089,  6091,  6101,  6113,  6121,  6131,  6133,
    6143,  6151,  6163,  6173,  6197,  6199,  6203,  6211,  6217,  6221,  6229,  6247,  6257,  6263,  6269,  6271,
    6277,  6287,  6299,  6301,  6311,  6317,  6323,  6329,  6337,  6343,  6353,  6359,  6361,  6367,  6373,  6379,
    6389,  6397,  6421,  6427,  6449,  6451,  6469,  6473,  6481,  6491,  6521,  6529,  6547,  6551,  6553,  6563,
    6569,  6571,  6577,  6581,  6599,  6607,  6619,  6637,  6653,  6659,  6661,  6673,  6679,  6689,  6691,  6701,
    6703,  6709,  6719,  6733,  6737,  6761,  6763,  6779,  6781,  6791,  6793,  6803,  6823,  6827,  6829,  6833,
    6841,  6857,  6863,  6869,  6871,  6883,  6899,  6907,  6911,  6917,  6947,  6949,  6959,  6961,  6967,  6971,
    6977,  6983,  6991,  6997,  7001,  7013,  7019,  7027,  7039,  7043,  7057,  7069,  7079,  7103,  7109,  7121,
    7127,  7129,  7151,  7159,  7177,  7187,  7193,  7207,  7211,  7213,  7219,  7229,  7237,  7243,  7247,  7253,
    7283,  7297,  7307,  7309,  7321,  7331,  7333,  7349,  7351,  7369,  7393,  7411,  7417,  7433,  7451,  7457,
    7459,  7477,  7481,  7487,  7489,  7499,  7507,  7517,  7523,  7529,  7537,  7541,  7547,  7549,  7559,  7561,
    7573,  7577,  7583,  7589,  7591,  7603,  7607,  7621,  7639,  7643,  7649,  7669,  7673,  7681,  7687,  7691,
    7699,  7703,  7717,  7723,  7727,  7741,  7753,  7757,  7759,  7789,  7793,  7817,  7823,  7829,  7841,  7853,
    7867,  7873,  7877,  7879,  7883,  7901,  7907,  7919,  7927,  7933,  7937,  7949,  7951,  7963,  7993,  8009,
    8011,  8017,  8039,  8053,  8059,  8069,  8081,  8087,  8089,  8093,  8101,  8111,  8117,  8123,  8147,  8161,
    8167,  8171,  8179,  8191,  8209,  8219,  8221,  8231,  8233,  8237,  8243,  8263,  8269,  8273,  8287,  8291,
    8293,  8297,  8311,  8317,  8329,  8353,  8363,  8369,  8377,  8387,  8389,  8419,  8423,  8429,  8431,  8443,
    8447,  8461,  8467,  8501,  8513,  8521,  8527,  8537,  8539,  8543,  8563,  8573,  8581,  8597,  8599,  8609,
    8623,  8627,  8629,  8641,  8647,  8663,  8669,  8677,  8681,  8689,  8693,  8699,  8707,  8713,  8719,  8731,
    8737,  8741,  8747,  8753,  8761,  8779,  8783,  8803,  8807,  8819,  8821,  8831,  8837,  8839,  8849,  8861,
    8863,  8867,  8887,  8893,  8923,  8929,  8933,  8941,  8951,  8963,  8969,  8971,  8999,  9001,  9007,  9011,
    9013,  9029,  9041,  9043,  9049,  9059,  9067,  9091,  9103,  9109,  9127,  9133,  9137,  9151,  9157,  9161,
    9173,  9181,  9187,  9199,  9203,  9209,  9221,  9227,  9239,  9241,  9257,  9277,  9281,  9283,  9293,  9311,
    9319,  9323,  9337,  9341,  9343,  9349,  9371,  9377,  9391,  9397,  9403,  9413,  9419,  9421,  9431,  9433,
    9437,  9439,  9461,  9463,  9467,  9473,  9479,  9491,  9497,  9511,  9521,  9533,  9539,  9547,  9551,  9587,
    9601,  9613,  9619,  9623,  9629,  9631,  9643,  9649,  9661,  9677,  9679,  9689,  9697,  9719,  9721,  9733,
    9739,  9743,  9749,  9767,  9769,  9781,  9787,  9791,  9803,  9811,  9817,  9829,  9833,  9839,  9851,  9857,
    9859,  9871,  9883,  9887,  9901,  9907,  9923,  9929,  9931,  9941,  9949,  9967,  9973,  10007, 10009, 10037,
    10039, 10061, 10067, 10069, 10079, 10091, 10093, 10099, 10103, 10111, 10133, 10139, 10141, 10151, 10159, 10163,
    10169, 10177, 10181, 10193, 10211, 10223, 10243, 10247, 10253, 10259, 10267, 10271, 10273, 10289, 10301, 10303,
    10313, 10321, 10331, 10333, 10337, 10343, 10357, 10369, 10391, 10399, 10427, 10429, 10433, 10453, 10457, 10459,
    10463, 10477, 10487, 10499, 10501, 10513, 10529, 10531, 10559, 10567, 10589, 10597, 10601, 10607, 10613, 10627,
    10631, 10639, 10651, 10657, 10663, 10667, 10687, 10691, 10709, 10711, 10723, 10729, 10733, 10739, 10753, 10771,
    10781, 10789, 10799, 10831, 10837, 10847, 10853, 10859, 10861, 10867, 10883, 10889, 10891, 10903, 10909, 10937,
    10939, 10949, 10957, 10973, 10979, 10987, 10993, 11003, 11027, 11047, 11057, 11059, 11069, 11071, 11083, 11087,
    11093, 11113, 11117, 11119, 11131, 11149, 11159, 11161, 11171, 11173, 11177, 11197, 11213, 11239, 11243, 11251,
    11257, 11261, 11273, 11279, 11287, 11299, 11311, 11317, 11321, 11329, 11351, 11353, 11369, 11383, 11393, 11399,
    11411, 11423, 11437, 11443, 11447, 11467, 11471, 11483, 11489, 11491, 11497, 11503, 11519, 11527, 11549, 11551,
    11579, 11587, 11593, 11597, 11617, 11621, 11633, 11657, 11677, 11681, 11689, 11699, 11701, 11717, 11719, 11731,
    11743, 11777, 11779, 11783, 11789, 11801, 11807, 11813, 11821, 11827, 11831, 11833, 11839, 11863, 11867, 11887,
    11897, 11903, 11909, 11923, 11927, 11933, 11939, 11941, 11953, 11959, 11969, 11971, 11981, 11987, 12007, 12011,
    12037, 12041, 12043, 12049, 12071, 12073, 12097, 12101, 12107, 12109, 12113, 12119, 12143, 12149, 12157, 12161,
    12163, 12197, 12203, 12211, 12227, 12239, 12241, 12251, 12253, 12263, 12269, 12277, 12281, 12289, 12301, 12323,
    12329, 12343, 12347, 12373, 12377, 12379, 12391, 12401, 12409, 12413, 12421, 12433, 12437, 12451, 12457, 12473,
    12479, 12487, 12491, 12497, 12503, 12511, 12517, 12527, 12539, 12541, 12547, 12553, 12569, 12577, 12583, 12589,
    12601, 12611, 12613, 12619, 12637, 12641, 12647, 12653, 12659, 12671, 12689, 12697, 12703, 12713, 12721, 12739,
    12743, 12757, 12763, 12781, 12791, 12799, 12809, 12821, 12823, 12829, 12841, 12853, 12889, 12893, 12899, 12907,
    12911, 12917, 12919, 12923, 12941, 12953, 12959, 12967, 12973, 12979, 12983, 13001, 13003, 13007, 13009, 13033,
    13037, 13043, 13049, 13063, 13093, 13099, 13103, 13109, 13121, 13127, 13147, 13151, 13159, 13163, 13171, 13177,
    13183, 13187, 13217, 13219, 13229, 13241, 13249, 13259, 13267, 13291, 13297, 13309, 13313, 13327, 13331, 13337,
    13339, 13367, 13381, 13397, 13399, 13411, 13417, 13421, 13441, 13451, 13457, 13463, 13469, 13477, 13487, 13499,
    13513, 13523, 13537, 13553, 13567, 13577, 13591, 13597, 13613, 13619, 13627, 13633, 13649, 13669, 13679, 13681,
    13687, 13691, 13693, 13697, 13709, 13711, 13721, 13723, 13729, 13751, 13757, 13759, 13763, 13781, 13789, 13799,
    13807, 13829, 13831, 13841, 13859, 13873, 13877, 13879, 13883, 13901, 13903, 13907, 13913, 13921, 13931, 13933,
    13963, 13967, 13997, 13999, 14009, 14011, 14029, 14033, 14051, 14057, 14071, 14081, 14083, 14087, 14107, 14143,
    14149, 14153, 14159, 14173, 14177, 14197, 14207, 14221, 14243, 14249, 14251, 14281, 14293, 14303, 14321, 14323,
    14327, 14341, 14347, 14369, 14387, 14389, 14401, 14407, 14411, 14419, 14423, 14431, 14437, 14447, 14449, 14461,
    14479, 14489, 14503, 14519, 14533, 14537, 14543, 14549, 14551, 14557, 14561, 14563, 14591, 14593, 14621, 14627,
    14629, 14633, 14639, 14653, 14657, 14669, 14683, 14699, 14713, 14717, 14723, 14731, 14737, 14741, 14747, 14753,
    14759, 14767, 14771, 14779, 14783, 14797, 14813, 14821, 14827, 14831, 14843, 14851, 14867, 14869, 14879, 14887,
    14891, 14897, 14923, 14929, 14939, 14947, 14951, 14957, 14969, 14983, 15013, 15017, 15031, 15053, 15061, 15073,
    15077, 15083, 15091, 15101, 15107, 15121, 15131, 15137, 15139, 15149, 15161, 15173, 15187, 15193, 15199, 15217,
    15227, 15233, 15241, 15259, 15263, 15269, 15271, 15277, 15287, 15289, 15299, 15307, 15313, 15319, 15329, 15331,
    15349, 15359, 15361, 15373, 15377, 15383, 15391, 15401, 15413, 15427, 15439, 15443, 15451, 15461, 15467, 15473,
    15493, 15497, 15511, 15527, 15541, 15551, 15559, 15569, 15581, 15583, 15601, 15607, 15619, 15629, 15641, 15643,
    15647, 15649, 15661, 15667, 15671, 15679, 15683, 15727, 15731, 15733, 15737, 15739, 15749, 15761, 15767, 15773,
    15787, 15791, 15797, 15803, 15809, 15817, 15823, 15859, 15877, 15881, 15887, 15889, 15901, 15907, 15913, 15919,
    15923, 15937, 15959, 15971, 15973, 15991, 16001, 16007, 16033, 16057, 16061, 16063, 16067, 16069, 16073, 16087,
    16091, 16097, 16103, 16111, 16127, 16139, 16141, 16183, 16187, 16189, 16193, 16217, 16223, 16229, 16231, 16249,
    16253, 16267, 16273, 16301, 16319, 16333, 16339, 16349, 16361, 16363, 16369, 16381, 16411, 16417, 16421, 16427,
    16433, 16447, 16451, 16453, 16477, 16481, 16487, 16493, 16519, 16529, 16547, 16553, 16561, 16567, 16573, 16603,
    16607, 16619, 16631, 16633, 16649, 16651, 16657, 16661, 16673, 16691, 16693, 16699, 16703, 16729, 16741, 16747,
    16759, 16763, 16787, 16811, 16823, 16829, 16831, 16843, 16871, 16879, 16883, 16889, 16901, 16903, 16921, 16927,
    16931, 16937, 16943, 16963, 16979, 16981, 16987, 16993, 17011, 17021, 17027, 17029, 17033, 17041, 17047, 17053,
    17077, 17093, 17099, 17107, 17117, 17123, 17137, 17159, 17167, 17183, 17189, 17191, 17203, 17207, 17209, 17231,
    17239, 17257, 17291, 17293, 17299, 17317, 17321, 17327, 17333, 17341, 17351, 17359, 17377, 17383, 17387, 17389,
    17393, 17401, 17417, 17419, 17431, 17443, 17449, 17467, 17471, 17477, 17483, 17489, 17491, 17497, 17509, 17519,
    17539, 17551, 17569, 17573, 17579, 17581, 17597, 17599, 17609, 17623, 17627, 17657, 17659, 17669, 17681, 17683,
    17707, 17713, 17729, 17737, 17747, 17749, 17761, 17783, 17789, 17791, 17807, 17827, 17837, 17839, 17851, 17863,
};
static const unsigned int known_primes_size = (unsigned int)(sizeof(known_primes) / sizeof(known_primes[0]));
MPN_STATIC_ASSERT(sizeof(mpn_limb_t) >= sizeof(uint32_t), "under this implementation, sizeof(mpn_limb_t) MUST not smaller than sizeof(uint32_t)");
// clang-format on

static int random_with_pattern_bin(mpn_limb_t *r, unsigned int bits, unsigned int top, unsigned int bottom,
                                   int (*rand_bytes)(void *, unsigned char *, unsigned int), void *rand_state)
{
    if (bits == 0) { return 0; }
    if (top > bits || bottom > bits) { return -EINVAL; }
    if (top > MPN_LIMB_BITS || bottom > MPN_LIMB_BITS) { return -EINVAL; }

    int err = 0;
    unsigned int words = MPN_BITS_TO_LIMBS(bits);

    if ((err = rand_bytes(rand_state, (unsigned char *)r, words * MPN_LIMB_BYTES)) != 0) {
        MPI_RAISE_ERROR(-EACCES, "RNG failed.");
        return err;
    }
    r[words - 1] &= MPN_MASK_LIMB_HI(bits);

    if (top > 0) {
        unsigned int n = bits & (MPN_LIMB_BITS - 1);
        if (n == 0) {
            r[words - 1] |= MPN_MASK_LIMB_HI(top);
        } else {
            if (n > top) {
                r[words - 1] |= (((mpn_limb_t)1 << top) - 1) << ((n & (MPN_LIMB_BITS - 1)) - top);
            } else {
                r[words - 1] = MPN_MASK_LIMB_HI(bits);
                r[words - 2] |= MPN_MASK_LIMB_HI(top - n);
            }
        }
    }
    if (bottom > 0) { r[0] |= (((mpn_limb_t)1 << bottom) - 1); }

    return 0;
}

/* return MPN_LIMB_MASK if have more than one lime; otherwise, return the first limb */
MPN_INLINE mpn_limb_t mpi_get_limb(const mpi_t *a)
{
    MPN_ASSERT(a != NULL);
    if (a->size > 1) {
        return MPN_LIMB_MASK;
    } else if (a->size == 1) {
        return a->data[0];
    } else {
        return 0;
    }
}

#define square(x) ((mpn_limb_t)(x) * (mpn_limb_t)(x))
static int generate_probable_prime(mpi_t *rnd, unsigned int bits, unsigned safe,
                                   int (*rand_bytes)(void *, unsigned char *, unsigned int), void *rand_state)
{
    uint32_t *mods = (uint32_t *)MPI_ZALLOCATE(sizeof(uint32_t), known_primes_size);
    if (mods == NULL) { return -ENOMEM; }

    int err;
    mpn_limb_t delta = 0;
    const mpn_limb_t maxdelta = MPN_LIMB_MASK - known_primes[known_primes_size - 1];

regenerate:
    if ((err = random_with_pattern_bin(rnd->data, bits, 2, 1, rand_bytes, rand_state)) != 0) {
        MPI_DEALLOCATE(mods);
        return err;
    }
    rnd->size = mpn_limbs(rnd->data, MPN_BITS_TO_LIMBS(bits));
    if (safe && (err = mpi_set_bit(rnd, 1)) != 0) {
        MPI_DEALLOCATE(mods);
        return err;
    }

    /* we now have a random number 'rnd' to test. */
    for (unsigned i = 1; i < known_primes_size; i++) {
        mpn_limb_t mod = mpi_mod_limb(rnd, (mpn_limb_t)known_primes[i]);
        if (mod == (mpn_limb_t)-1) {
            MPI_DEALLOCATE(mods);
            return -ERANGE;
        }
        mods[i] = (uint32_t)mod;
    }

recheck:
    for (unsigned i = 1; i < known_primes_size; i++) {
        /**
         * check that rnd is a prime and also that gcd(rnd-1,primes) == 1 (except for 2)
         * do the second check only if we are interested in safe primes
         * in the case that the candidate prime is a single word then
         * we check only the primes up to sqrt(rnd)
         */
        if (bits <= 31 && delta <= 0x7fffffff && square(known_primes[i]) > mpi_get_limb(rnd) + delta) {
            MPI_TRACE("check done");
            break;
        }
        if ((mods[i] + delta) % known_primes[i] <= (safe ? 1 : 0)) {
            delta += safe ? 4 : 2;
            if (delta > maxdelta) {
                MPI_TRACE("re-generate");
                goto regenerate;
            }
            MPI_TRACE("re-check");
            goto recheck;
        }
    }

    if ((err = mpi_add_limb(rnd, rnd, delta)) != 0) { return err; }
    if (mpi_bits(rnd) != bits) { goto regenerate; }

    MPI_DEALLOCATE(mods);

    return 0;
}

static int generate_probable_prime_dh(mpi_t *r, unsigned int bits, unsigned safe, const mpi_t *add, const mpi_t *rem,
                                      mpn_optimizer_t *optimizer,
                                      int (*rand_bytes)(void *, unsigned char *, unsigned int), void *rand_state)
{
    int ret = 0;
    mpn_limb_t delta;
    mpn_limb_t maxdelta = MPN_LIMB_MASK - known_primes[known_primes_size - 1];

    uint32_t *mods = (uint32_t *)MPI_ZALLOCATE(sizeof(uint32_t), known_primes_size);
    if (mods == NULL) { goto exit_with_error; }

    mpi_t *t1 = mpi_optimizer_get(optimizer, add->size);
    if (t1 == NULL) { return -ENOMEM; }

    delta = MPN_LIMB_MASK - mpi_get_limb(add);
    if (maxdelta > delta) { maxdelta = delta; }

again:
    if (random_with_pattern_bin(r->data, bits, 1, 1, rand_bytes, rand_state) != 0) { goto err; }

    /* (r - rem) % add == 0 required */
    if (mpi_mod(t1, r, add) != 0) { goto err; }
    if (mpi_sub(r, r, t1) != 0) { goto err; }
    if (rem == NULL) {
        if (mpi_add_limb(r, r, safe ? 3 : 1) != 0) { goto err; }
    } else {
        if (mpi_add(r, r, rem) != 0) { goto err; }
    }

    if (mpi_bits(r) < bits || mpi_get_limb(r) < (safe ? 5u : 3u)) {
        if (mpi_add(r, r, add) != 0) { goto err; }
    }

    /* we now have a random number 'r' to test. */
    for (unsigned int i = 1; i < known_primes_size; i++) {
        mpn_limb_t mod = mpi_mod_limb(r, (mpn_limb_t)known_primes[i]);
        if (mod == (mpn_limb_t)-1) goto err;
        mods[i] = (uint32_t)mod;
    }
    delta = 0;

loop:
    for (unsigned int i = 1; i < known_primes_size; i++) {
        /* check that r is a prime */
        if (bits <= 31 && delta <= 0x7fffffff && square(known_primes[i]) > r->data[0] + delta) break;
        /* r mod p == 1 implies q = (r-1)/2 is divisible by p */
        if (safe ? (mods[i] + delta) % known_primes[i] <= 1 : (mods[i] + delta) % known_primes[i] == 0) {
            delta += mpi_get_limb(add);
            if (delta > maxdelta) { goto again; }
            goto loop;
        }
    }
    if (mpi_add_limb(r, r, delta) != 0) { goto err; }
    ret = 1;

err:
    mpn_optimizer_put_limbs(optimizer, add->size);

    return ret;

exit_with_error:
    MPI_DEALLOCATE(mods);
    return 0;
}

/* miller-rabin checks for specified bits */
MPN_INLINE unsigned int prime_checks_for_bits(unsigned int bits)
{
    MPN_ASSERT(bits >= 6);

    const struct {
        unsigned int bits;
        unsigned int checks;
    } chocies[] = {{3747, 3}, {1345, 4}, {476, 5}, {400, 6}, {347, 7}, {308, 8}, {55, 27}, {6, 34}};
    const unsigned int count = (unsigned int)(sizeof(chocies) / sizeof(chocies[0]));

    for (unsigned int i = 0; i < count; i++) {
        if (bits > chocies[i].bits) { return chocies[i].checks; }
    }

    /* should never hit */
    MPI_RAISE_EXCEPTION();
    return chocies[count - 1].checks;
}

static int millerrabin_witness(mpi_t *witness, mpi_t *a1, mpi_t *a1_odd, unsigned int k, mpn_montgomery_t *mont)
{
    int err;
    /* witness = witness^a1_odd mod a */
    if ((err = mpi_montgomery_exp(witness, witness, a1_odd, mont)) != 0) {
        MPI_RAISE_ERROR(err);
        return err;
    }

    if (mpi_bits(witness) == 1) { return 1; /* probably prime */ }
    if (mpn_cmp(witness->data, witness->size, a1->data, a1->size) == 0) {
        return 1; /* witness == -1 (mod a), 'a' is probably prime */
    }

    while (--k > 0) {
        /* witness := witness^2 mod a */
        mpn_montgomery_square(witness->data, witness->data, mont);
        witness->size = mpn_limbs(witness->data, witness->size);

        if (mpi_bits(witness) == 1) {
            MPI_RAISE_ERROR(-EINVAL, "witness is composite");
            return 0; /* 'a' is composite, otherwise a previous 'witness' would have been == -1 (mod 'a') */
        }

        if (mpn_cmp(witness->data, witness->size, a1->data, a1->size) == 0) {
            return 1; /* witness == -1 (mod a), 'a' is probably prime */
        }
    }
    /**
     * If we get here, 'w' is the (a-1)/2-th power of the original 'w',
     * and it is neither -1 nor +1 -- so 'a' cannot be prime
     */
    return 0;
}

/**
 * mpi prime: test if a is a prime(Miller-Rabin Test)
 *
 * @note:
 *   1. return 0 if the number is composite
 *      1 if it is prime with an error probability of less than 0.25^checks
 */
int mpi_is_prime(const mpi_t *a, unsigned int checks, unsigned do_trial_division, mpn_optimizer_t *optimizer,
                 int (*rand_bytes)(void *, unsigned char *, unsigned int), void *rand_state)
{
    if (a == NULL) {
        MPI_RAISE_ERROR(-EINVAL, "Invalid Integer: nullptr");
        return -EINVAL;
    }

    /* prime must be odd and bigger than 1 */
    if (mpi_get_bit(a, 0) == 0 || mpi_get_limb(a) <= 1) {
        MPI_RAISE_WARN("prime must be odd and bigger than 1");
        return 0;
    }

    /* binary seach: check if a small prime */
    if (UNLIKELY(mpi_bytes(a) <= sizeof(uint32_t))) {
        mpn_limb_t aa = a->data[0];
        unsigned int lo = 0, hi = known_primes_size - 1;
        while (lo <= hi) {
            unsigned int mid = lo + (hi - lo) / 2;
            if (known_primes[mid] > aa) {
                hi = mid - 1;
            } else if (known_primes[mid] < aa) {
                lo = mid + 1;
            } else {
                return 1;
            }
        }
    }

    /* first look for small factors */
    if (do_trial_division) {
        for (unsigned int i = 1; i < known_primes_size; i++) {
            mpn_limb_t mod = mpi_mod_limb(a, known_primes[i]);
            if (mod == (mpn_limb_t)-1) {
                MPI_RAISE_ERROR(-ERANGE);
                return -ERANGE;
            }
            if (mod == 0) {
                MPI_RAISE_WARN("not a prime, at least one factor found: %u", known_primes[i]);
                return 0;
            }
        }
    }

    mpn_optimizer_t *opt = optimizer;
    if (opt == NULL) {
        opt = mpn_optimizer_create((a->size + MPI_ALIGNED_HEAD_LIMBS) * 3); /* a1, a1_odd, witness */
        if (opt == NULL) {
            MPI_RAISE_ERROR(-ENOMEM);
            return -ENOMEM;
        }
    }

    int err;
    mpn_montgomery_t *mont = NULL;
    mpi_t *a1 = mpi_optimizer_get(opt, a->size);
    mpi_t *a1_odd = mpi_optimizer_get(opt, a->size);
    mpi_t *witness = mpi_optimizer_get(opt, a->size);
    if (a1 == NULL || a1_odd == NULL || witness == NULL) {
        MPI_RAISE_ERROR(-ENOMEM);
        err = -ENOMEM;
        goto exit_with_error;
    }

    if ((err = mpi_sub_limb(a1, a, 1)) != 0) {
        MPI_RAISE_ERROR(err);
        goto exit_with_error;
    }

    /* define a1 = a1_odd * 2 ^ k */
    unsigned int k = 1;
    while (mpi_get_bit(a1, k) == 0) { k++; }
    if ((err = mpi_rshift(a1_odd, a1, k)) != 0) {
        MPI_RAISE_ERROR(err);
        goto exit_with_error;
    }

    /* Montgomery setup for computations mod a */
    mont = mpn_montgomery_create(mpi_bits(a), 6 * a->size);
    if (mont == NULL) {
        MPI_RAISE_ERROR(-ENOMEM);
        goto exit_with_error;
    }
    if ((err = mpi_montgomery_set_modulus(mont, a)) != 0) {
        MPI_RAISE_ERROR(err);
        goto exit_with_error;
    }

    if (checks == 0) { checks = prime_checks_for_bits(mpi_bits(a)); }
    for (unsigned int i = 0; i < checks; i++) {
        /* 1 < witness < a-1 */
        mpn_limb_t one = 1;
        if ((err = mpn_random_range(witness->data, 1000, &one, 1, a1->data, a1->size, rand_bytes, rand_state)) != 0) {
            MPI_RAISE_ERROR(err);
            goto exit_with_error;
        }
        witness->size = mpn_limbs(witness->data, a1->size);

        /* miller-rabin test */
        err = millerrabin_witness(witness, a1, a1_odd, k, mont);
        if (err <= 0) { goto exit_with_error; }
    }

exit_with_error:
    if (witness != NULL) { mpi_optimizer_put(opt, a->size); }
    if (a1_odd != NULL) { mpi_optimizer_put(opt, a->size); }
    if (a1 != NULL) { mpi_optimizer_put(opt, a->size); }
    if (optimizer == NULL && opt != NULL) { mpn_optimizer_destory(opt); }
    mpn_montgomery_destory(mont);

    return err;
}

/**
 * mpi prime: enerates a pseudo-random prime number of at least bit length |bits|
 *
 * @note:
 *   1. The returned number is probably prime with a negligible error.
 *   2. If |add| is NULL the returned prime number will have exact bit length |bits| with the top most two
 * bits set.
 *   3. The prime may have to fulfill additional requirements for use in Diffie-Hellman key exchange:
 *      If |add| is not NULL, the prime will fulfill the condition p % |add| == |rem| (p % |add| == 1 if
 * |rem| == NULL) in order to suit a given generator.
 *
 *      If |safe| is true, it will be a safe prime (i.e. a prime p so hat (p-1)/2 is also prime).
 *      If |safe| is true, and |rem| == NULL the condition will be p % |add| == 3.
 *      It is recommended that |add| is a multiple of 4.
 */
int mpi_generate_prime(mpi_t *ret, unsigned int bits, unsigned safe, const mpi_t *add, const mpi_t *rem,
                       int (*rand_bytes)(void *, unsigned char *, unsigned int), void *rand_state)
{
    if (bits < 2) {
        MPI_RAISE_ERROR(-EINVAL, "No prime numbers for given bits");
        return -EINVAL;
    } else if (add == NULL && safe && bits < 6 && bits != 3) {
        /**
         * the smallest safe prime (7) is three bits
         * but the following two safe primes with less than 6 bits (11, 23)
         * are unreachable for random_with_pattern_bin withe |top| == 2
         */
        return -EINVAL;
    }

    int err = 0;

    mpn_optimizer_t *optimizer = NULL;
    {
        /* create proper required optimizer once */
        unsigned int optsize = 0;
        optsize += safe ? (MPI_ALIGNED_HEAD_LIMBS + MPN_BITS_TO_LIMBS(bits)) : 0; /* t */
        optsize += add != NULL ? (MPI_ALIGNED_HEAD_LIMBS + add->size) : 0;        /* generate_probable_prime_dh */
        optsize += (MPI_ALIGNED_HEAD_LIMBS + MPN_BITS_TO_LIMBS(bits)) * 3;        /* mpi_is_prime */

        optimizer = mpn_optimizer_create(optsize);
        if (optimizer == NULL) {
            MPI_RAISE_ERROR(-ENOMEM);
            err = -ENOMEM;
            goto exit_with_error;
        }
    }
    MPN_ASSERT(optimizer != NULL);

    mpi_t *t = NULL;
    if (safe) {
        t = mpi_optimizer_get(optimizer, MPN_BITS_TO_LIMBS(bits));
        if (t == NULL) {
            MPI_RAISE_ERROR(-ENOMEM);
            err = -ENOMEM;
            goto exit_with_error;
        }
    }

    // unsigned int tries = 0;
regenerate:
    // if (tries++ > 1000) { MPI_RAISE_WARN("generate prime: tries %u", tries); }
    /* generate a random number and set the top and bottom bits */
    if (add == NULL) {
        if ((err = generate_probable_prime(ret, bits, safe, rand_bytes, rand_state)) != 0) {
            MPI_RAISE_ERROR(err);
            goto exit_with_error;
        }
    } else {
        if ((err = generate_probable_prime_dh(ret, bits, safe, add, rem, optimizer, rand_bytes, rand_state)) != 0) {
            MPI_RAISE_ERROR(err);
            goto exit_with_error;
        }
    }

    unsigned int checks = prime_checks_for_bits(bits);
    if (!safe) {
        err = mpi_is_prime(ret, checks, 0, optimizer, rand_bytes, rand_state);
        if (err < 0) {
            MPI_RAISE_ERROR(err);
            goto exit_with_error;
        }
        if (err == 0) { goto regenerate; }
    } else {
        /* for "safe prime" generation, check that (p - 1) / 2 is prime. prime is odd, so just right-shift
         */
        if ((err = mpi_rshift(t, ret, 1)) != 0) {
            MPI_RAISE_ERROR(err);
            goto exit_with_error;
        }

        for (unsigned int i = 0; i < checks; i++) {
            err = mpi_is_prime(ret, 1, 0, optimizer, rand_bytes, rand_state);
            if (err < 0) {
                MPI_RAISE_ERROR(err);
                goto exit_with_error;
            }
            if (err == 0) { goto regenerate; }

            err = mpi_is_prime(t, 1, 0, optimizer, rand_bytes, rand_state);
            if (err < 0) {
                MPI_RAISE_ERROR(err);
                goto exit_with_error;
            }
            if (err == 0) { goto regenerate; }
        }
    }
    err = 0;

exit_with_error:
    mpn_optimizer_destory(optimizer);

    return err;
}

/**
 * test if |a| and |b| are coprime
 */
int mpn_is_coprime(mpn_limb_t *a, unsigned int asize, mpn_limb_t *b, unsigned int bsize, mpn_optimizer_t *optimizer)
{
    if (asize > bsize) {
        SWAP(mpn_limb_t *, a, b);
        SWAP(unsigned int, asize, bsize);
    }

    mpi_t *r = mpi_optimizer_get(optimizer, 64);
    MPN_ASSERT(r != NULL);

    mpi_t ta, tb;
    mpi_make(&ta, a, asize);
    mpi_make(&tb, b, bsize);

    mpi_gcd_consttime(r, &ta, &tb, optimizer);
    int ret = r->size == 1 && r->data[0] == 1;
    mpi_optimizer_put(optimizer, 64);

    return ret;
}
