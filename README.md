# libmpi

[![license](https://img.shields.io/badge/license-Apache-brightgreen.svg?style=flat)](https://github.com/vxfury/libmpi/blob/master/LICENSE)
[![CI Status](https://github.com/vxfury/libmpi/workflows/ci/badge.svg)](https://github.com/vxfury/libmpi/actions)
[![codecov](https://codecov.io/gh/vxfury/libmpi/branch/main/graph/badge.svg?token=5IfLTTEcnF)](https://codecov.io/gh/vxfury/libmpi)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/vxfury/libmpi?color=red&label=release)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/vxfury/libmpi/pulls)

Multiple Precision Integer and Relevant Algorithms, such as Bignum, RSA, DH, ECDH, ECDSA
## Benchmark(libmpi VS openssl)

|               operation               | average time(nanoseconds) |    reference     | coefficient of variation | perfermance ratio |
| :-: | :-: | :-: | :-: | :-: |
| from-string(mpi vs ossl)              |        2130.387600        |   24357.714800   |         0.021688         | <span style="color:#8000;">**11.433466**</span>         |
| to-string(mpi vs ossl)                |        572.967400         |   2252.049000    |         0.041777         | <span style="color:#8000;">**3.930501**</span>          |
| from-octets(mpi vs ossl)              |        239.663200         |    550.247000    |         0.064596         | <span style="color:#8000;">**2.295918**</span>          |
| to-octets(mpi vs ossl)                |        114.321600         |   1281.856600    |         0.093529         | <span style="color:#8000;">**11.212724**</span>         |
| add(mpi vs ossl)                      |         70.921000         |    222.582800    |         0.118746         | <span style="color:#8000;">**3.138461**</span>          |
| add-assign(mpi vs ossl)               |         68.280800         |    224.943000    |         0.121105         | <span style="color:#8000;">**3.294381**</span>          |
| sub(mpi vs ossl)                      |         54.840800         |    142.361800    |         0.135038         | <span style="color:#8000;">**2.595910**</span>          |
| sub-assign(mpi vs ossl)               |         61.460800         |    226.182800    |         0.127575         | <span style="color:#8000;">**3.680115**</span>          |
| mul(mpi vs ossl)                      |        1606.460800        |   9894.867800    |         0.025212         | <span style="color:#8000;">**6.159421**</span>          |
| sqr(mpi vs ossl)                      |        994.632800         |   6913.709400    |         0.033124         | <span style="color:#8000;">**6.951017**</span>          |
| div(mpi vs ossl)                      |        3102.480000        |   31338.845000   |         0.017963         | <span style="color:#8000;">**10.101224**</span>         |
| gcd_consttime(mpi vs ossl)            |      3861233.896000       | 11951306.950000  |         0.000766         | <span style="color:#8000;">**3.095204**</span>          |
| montgomery-exp(mpi vs ossl)           |      8534925.745455       | 107564750.200000 |         0.000991         | <span style="color:#8000;">**12.602892**</span>         |
| montgomery-exp-consttime(mpi vs ossl) |      11866349.400000      | 114737603.800000 |         0.007274         | <span style="color:#8000;">**9.669158**</span>          |
| <span style="font-style:italic;">MUL2(a * 2 = a + a)</span>                   |         <span style="font-style:italic;">46.860400</span>         |   <span style="font-style:italic;">No Reference</span>   |         <span style="font-style:italic;">0.146086</span>         | <span style="font-style:italic;">N/A</span>               |
| <span style="font-style:italic;">MUL2(a * 2 = a << 1)</span>                  |         <span style="font-style:italic;">64.180600</span>         |   <span style="font-style:italic;">No Reference</span>   |         <span style="font-style:italic;">0.134770</span>         | <span style="font-style:italic;">N/A</span>               |
