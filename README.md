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
|       from-string(mpi vs ossl)        |   1641.51    |         22923.3         |        0.0296338         |      <span style="color:#8000;">**13.9647**</span>      |
|        to-string(mpi vs ossl)         |   559.044    |         2237.63         |         0.107149         |      <span style="color:#8000;">**4.00261**</span>      |
|       from-octets(mpi vs ossl)        |   182.501    |         524.403         |         0.100012         |      <span style="color:#8000;">**2.87342**</span>      |
|        to-octets(mpi vs ossl)         |   113.061    |         1176.89         |         0.119872         |      <span style="color:#8000;">**10.4093**</span>      |
|           add(mpi vs ossl)            |   45.2602    |         227.341         |         0.207147         |      <span style="color:#8000;">**5.02299**</span>      |
|        add-assign(mpi vs ossl)        |   40.7604    |         223.821         |         0.156637         |      <span style="color:#8000;">**5.49115**</span>      |
|           sub(mpi vs ossl)            |   57.4404    |         132.001         |         0.214028         |      <span style="color:#8000;">**2.29805**</span>      |
|        sub-assign(mpi vs ossl)        |   53.2004    |         223.482         |         0.231139         |      <span style="color:#8000;">**4.20075**</span>      |
|           mul(mpi vs ossl)            |   1590.63    |         9081.66         |        0.0733495         |      <span style="color:#8000;">**5.70947**</span>      |
|           sqr(mpi vs ossl)            |   864.846    |         6224.38         |        0.0834486         |      <span style="color:#8000;">**7.1971**</span>       |
|           div(mpi vs ossl)            |   2756.74    |          29840          |        0.0432187         |      <span style="color:#8000;">**10.8244**</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.88927e+06  |       1.18817e+07       |        0.0342213         |       <span style="color:#8000;">**3.055**</span>       |
|      montgomery-exp(mpi vs ossl)      | 7.82867e+06  |       9.6008e+07        |        0.0130853         |      <span style="color:#8000;">**12.2636**</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.16155e+07  |       1.02111e+08       |        0.0628727         |      <span style="color:#8000;">**8.79093**</span>      |
|          MUL2(a * 2 = a + a)          |   18.0602    |      <span style="font-style:italic;">No Reference</span>       |         0.431107         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   39.2804    |      <span style="font-style:italic;">No Reference</span>       |         0.244427         |        N/A        |
