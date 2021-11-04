# libmpi

[![license](https://img.shields.io/badge/license-Apache-brightgreen.svg?style=flat)](https://github.com/vxfury/libmpi/blob/master/LICENSE)
[![CI Status](https://github.com/vxfury/libmpi/workflows/ci/badge.svg)](https://github.com/vxfury/libmpi/actions)
[![codecov](https://codecov.io/gh/vxfury/libmpi/branch/main/graph/badge.svg?token=5IfLTTEcnF)](https://codecov.io/gh/vxfury/libmpi)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/vxfury/libmpi?color=red&label=release)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/vxfury/libmpi/pulls)

Multiple Precision Integer and Relevant Algorithms, such as Bignum, RSA, DH, ECDH, ECDSA
## Benchmark(libmpi VS openssl)

|              description              | average time | average time(reference) | coefficient of variation | perfermance ratio |
| :-: | :-: | :-: | :-: | :-: |
|       from-string(mpi vs ossl)        |   2130.17    |         24447.8         |        0.0217243         |      <span style="color:#8000;">**11.4769**</span>      |
|        to-string(mpi vs ossl)         |   573.667    |         2239.87         |        0.0419025         |      <span style="color:#8000;">**3.90447**</span>      |
|       from-octets(mpi vs ossl)        |   246.083    |         550.467         |         0.101732         |      <span style="color:#8000;">**2.23691**</span>      |
|        to-octets(mpi vs ossl)         |   115.301    |         1275.24         |        0.0945009         |       <span style="color:#8000;">**11.06**</span>       |
|           add(mpi vs ossl)            |   69.9208    |         223.123         |         0.120458         |      <span style="color:#8000;">**3.19108**</span>      |
|        add-assign(mpi vs ossl)        |   68.5408    |         223.923         |         0.120791         |       <span style="color:#8000;">**3.267**</span>       |
|           sub(mpi vs ossl)            |   55.5406    |         142.222         |         0.134185         |      <span style="color:#8000;">**2.56068**</span>      |
|        sub-assign(mpi vs ossl)        |   61.2208    |         227.603         |         0.12782          |      <span style="color:#8000;">**3.71774**</span>      |
|           mul(mpi vs ossl)            |   1604.44    |         9996.02         |        0.0250431         |      <span style="color:#8000;">**6.23023**</span>      |
|           sqr(mpi vs ossl)            |   978.192    |         6947.5          |        0.0321178         |      <span style="color:#8000;">**7.10239**</span>      |
|           div(mpi vs ossl)            |    2998.5    |         31462.8         |        0.0183108         |      <span style="color:#8000;">**10.4929**</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.87052e+06  |       1.20682e+07       |        0.00112097        |      <span style="color:#8000;">**3.11799**</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.57905e+06  |       1.06573e+08       |       0.000351559        |      <span style="color:#8000;">**12.4224**</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.17921e+07  |       1.13463e+08       |       0.000325743        |      <span style="color:#8000;">**9.62201**</span>      |
|          MUL2(a * 2 = a + a)          |   28.5004    |      <span style="font-style:italic;">No Reference</span>       |         0.187395         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   46.1206    |      <span style="font-style:italic;">No Reference</span>       |         0.147252         |        N/A        |
