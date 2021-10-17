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
| from-string(mpi vs ossl)              |        2062.009000        |   24094.460400   |         0.026612         | <span style="color:#8000;">**11.684944**</span>         |
| to-string(mpi vs ossl)                |        589.828400         |   2243.831600    |         0.065074         | <span style="color:#8000;">**3.804211**</span>          |
| from-octets(mpi vs ossl)              |        184.542600         |    572.228200    |         0.075154         | <span style="color:#8000;">**3.100792**</span>          |
| to-octets(mpi vs ossl)                |        114.481600         |   1286.378200    |         0.093465         | <span style="color:#8000;">**11.236550**</span>         |
| add(mpi vs ossl)                      |         44.220600         |    220.983000    |         0.150382         | <span style="color:#8000;">**4.997286**</span>          |
| add-assign(mpi vs ossl)               |         44.460600         |    226.123200    |         0.150146         | <span style="color:#8000;">**5.085923**</span>          |
| sub(mpi vs ossl)                      |         66.721000         |    142.342000    |         0.500661         | <span style="color:#8000;">**2.133391**</span>          |
| sub-assign(mpi vs ossl)               |         51.040800         |    232.143200    |         0.140932         | <span style="color:#8000;">**4.548189**</span>          |
| mul(mpi vs ossl)                      |        1606.982600        |   9985.841000    |         0.025379         | <span style="color:#8000;">**6.214032**</span>          |
| sqr(mpi vs ossl)                      |        991.154000         |   6895.437400    |         0.031875         | <span style="color:#8000;">**6.956979**</span>          |
| div(mpi vs ossl)                      |        3418.548200        |   31261.561800   |         0.017128         | <span style="color:#8000;">**9.144689**</span>          |
| gcd_consttime(mpi vs ossl)            |      4303897.330435       | 11891873.000000  |         0.003858         | <span style="color:#8000;">**2.763048**</span>          |
| montgomery-exp(mpi vs ossl)           |      8679738.981818       | 106503744.800000 |         0.000416         | <span style="color:#8000;">**12.270386**</span>         |
| montgomery-exp-consttime(mpi vs ossl) |      11802136.550000      | 113588004.800000 |         0.000338         | <span style="color:#8000;">**9.624359**</span>          |
| MUL2(a * 2 = a + a)                   |         40.500600         |  <No Reference>  |         0.157134         | N/A               |
| MUL2(a * 2 = a << 1)                  |         74.101000         |  <No Reference>  |         0.116172         | N/A               |

